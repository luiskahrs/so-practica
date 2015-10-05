#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define MYPORT "7777"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold

struct Registration
{
	char multicastGroup[24];
	pid_t clientPid;
};

typedef struct {
	int id;
} t_identificador;

typedef struct {
	int id_proceso;
	int ruta_mCod_long;
	char* ruta_mCod;
	int proxima_sentencia;
	int quantum; // si es 0 es porque es FIFO
} t_msg_de_planificador;

typedef struct {
	int id_proceso;
	int motivo; // 0: error, 1: finalizo, 2: termino rafaga, 3: termino E/S
	int ultima_sentencia;
	int sleep;
	int mensaje_long;
	char* mensaje;
} t_msg_a_planificador;

typedef struct {
	pid_t clientPid;
	char cliente[5];
	char metodo[20];
	char message[1024];
} t_llamada;

void enviar_pcb_a_planificador(int fd, int id_proceso);
void PruebaMMap();
void PruebaMongoDB();
int PruebaSocket();
void sendall( int descriptorSocket, const char* buffer, const unsigned int bytesPorEnviar);
void PruebaCliente();
void PruebaServidor();
void ObtenerCurrentPath();
void connect_to_planificador();
void enviar_pcb_a_planificador(int fd, int id_proceso);

int main(int argc, char *argv[])
{
	//PruebaMMap();
	//PruebaMongoDB();
	//PruebaSocket();
	//PruebaCliente();
	//PruebaServidor();
	connect_to_planificador();
    return 0;
}

void PruebaMMap(){

    int fd, offset;
    char *data;
    struct stat sbuf;
    div_t cantBloques;
    int tamBloque=1024*1024*20;

    if ((fd = open("/home/utnso/workspace/so-practica/inputejemplo", O_RDONLY)) == -1) {
    	printf("%s", get_current_dir_name());
        perror("open");
        exit(1);
    }

    if (stat("/home/utnso/workspace/so-practica/inputejemplo", &sbuf) == -1) {
        perror("stat");
        exit(1);
    }

    offset = 0;
    printf("El tamaño del archivo es %i\n", (int) sbuf.st_size);
    if (offset < 0 || offset > sbuf.st_size-1) {
        fprintf(stderr, "mmapdemo: offset must be in the range 0-%d\n", sbuf.st_size-1);
        exit(1);
    }

    // el ultimo parametro es el desplazamiento
    data = mmap((caddr_t)0, tamBloque, PROT_READ, MAP_SHARED, fd, 0);

    if (data == (caddr_t)(-1)) {
        perror("mmap");
        exit(1);
    }

    cantBloques = div((int)sbuf.st_size, tamBloque);

    if (cantBloques.rem >0) {
    cantBloques.quot++;
    }
    printf("%d\n",  cantBloques.quot);

    printf("Imprimo lo que se encuentra dentro del archivo: %s", data);
}

void PruebaMongoDB()
{
	mongoc_collection_t *collection;
	   mongoc_client_t *client;
	   mongoc_cursor_t *cursor;
	   const bson_t *item;
	   bson_error_t error;
	   bson_oid_t oid;
	   bson_t *query;
	   bson_t *doc;
	   char *str;
	   bool r;

	   mongoc_init();

	   /* get a handle to our collection */
	   client = mongoc_client_new ("mongodb://localhost:27017");
	   collection = mongoc_client_get_collection (client, "local", "tito");

	   /* insert a document */
	    bson_oid_init (&oid, NULL);
	    doc = BCON_NEW ("_id", BCON_OID (&oid),
	                    "hello", BCON_UTF8 ("world!"));
	    r = mongoc_collection_insert (collection, MONGOC_INSERT_NONE, doc, NULL, &error);
	    if (!r) {
	       fprintf (stderr, "%s\n", error.message);
	    }

	    /* build a query to execute */
	    query = BCON_NEW ("_id", BCON_OID (&oid));

	    /* execute the query and iterate the results */
	    cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
	    while (mongoc_cursor_next (cursor, &item)) {
	       str = bson_as_json (item, NULL);
	       printf ("%s\n", str);
	       bson_free (str);
	    }

	    /* release everything */
	    mongoc_cursor_destroy (cursor);
	    mongoc_collection_destroy (collection);
	    mongoc_client_destroy (client);
	    bson_destroy (query);
	    bson_destroy (doc);
}

int PruebaSocket(){
	int sock_desc = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_desc == -1)
	{
		printf("cannot create socket!\n");
		return 0;
	}

	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(7777);

	if (bind(sock_desc, (struct sockaddr*)&server, sizeof(server)) != 0)
	{
		printf("cannot bind socket!\n");
		close(sock_desc);
		return 0;
	}

	if (listen(sock_desc, 20) != 0)
	{
		printf("cannot listen on socket!\n");
		close(sock_desc);
		return 0;
	}

	struct sockaddr_in client;
	memset(&client, 0, sizeof(client));
	socklen_t len = sizeof(client);
	int temp_sock_desc = accept(sock_desc, (struct sockaddr*)&client, &len);

	if (temp_sock_desc == -1)
	{
		printf("cannot accept client!\n");
		close(sock_desc);
		return 0;
	}

	char buf[100];
	int k;

	while(1)
	{
		printf("Trato de leer...\n");

		k = recv(temp_sock_desc, buf, 100, 0);
		if (k == -1)
		{
			printf("\ncannot read from client!\n");
			break;
		}

		if (k == 0)
		{
			printf("\nclient disconnected.\n");
			break;
		}

		if (k > 0)
			printf("%*.*s", k, k, buf);

		// remuevo el \n que me deja la lectura
		size_t ln = strlen(buf) - 1;
		if (buf[ln] == '\n')
			buf[ln] = '\0';

		printf("Esto es lo que hay en el buffer %s\n", buf);

		if (strcmp(buf, "exit") == 0)
			break;
	}

	close(temp_sock_desc);
	close(sock_desc);

	printf("server disconnected\n");
	return 0;
}

void sendall( int descriptorSocket, const char* buffer, const unsigned int bytesPorEnviar){
	int retorno;
	int bytesEnviados = 0;

	while (bytesEnviados < (int)bytesPorEnviar) {
	   retorno = send(descriptorSocket, (char*)(buffer+bytesEnviados), bytesPorEnviar-bytesEnviados, 0);

	   //Controlo Errores
	   if( retorno <= 0 ) {
		  printf("Error al enviar Datos (se corto el Paquete Enviado), solo se enviaron %d bytes de los %d bytes totales por enviar\n", bytesEnviados, (int)bytesPorEnviar);
		  perror("El Error Detectado es: ");
		  bytesEnviados = retorno;
		  break;
	   }
	   //Si no hay problemas, sigo acumulando bytesEnviados
	   bytesEnviados += retorno;
	}
}

void PruebaCliente()
{
	printf("Inicia main de un socket\n");
	struct addrinfo hints, *res;
	int sockfd;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	printf("Busca info del socket\n");
	getaddrinfo("192.168.1.104", "7777", &hints, &res);

	printf("Obtenida info del socket\n");

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	printf("Arme la estructura del socket\n");

	connect(sockfd, res->ai_addr, res->ai_addrlen);
	printf("Me conecte\n");
	unsigned int size = 1024; //Maximo
	char* buffer = malloc(size);
	int i;
	// Limpio el buffer cada vez que voy a leer
	for (i = 0; i < 1024; i++)
	{
		buffer[i] = '\0';
	}

	struct Registration regn ;
	regn.clientPid = getpid();
	strcpy(regn.multicastGroup, "Hola Victor!!!");

	printf("PID:%d\n", regn.clientPid);
	printf("MG:%s\n", regn.multicastGroup);
	printf("Size:%d\n", sizeof(regn));           //Size is 28

	char* data;
	data = (unsigned char*)malloc(sizeof(regn));
	memcpy(data, &regn, sizeof(regn));
	printf("Size:%d\n", sizeof(data));

	printf("Llego a mandar el socket");
	sendall(sockfd, data, sizeof(regn));
}

void PruebaServidor()
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd;
	t_llamada regn;

    // !! don't forget your error checking for these calls !!

    // first, load up address structs with getaddrinfo():

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    getaddrinfo(NULL, MYPORT, &hints, &res);

    // make a socket, bind it, and listen on it:

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);

    // now accept an incoming connection:

    addr_size = sizeof their_addr;

    printf("Llego a esperar la conexión\n");

    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    puts("Conectado...\n");
    // ready to communicate on socket descriptor new_fd!
    unsigned int size = 100000;
    char* buff = malloc(400000);

    if(recvfrom(new_fd, buff, size, 0, (struct sockaddr*)&their_addr, &addr_size) < 0)
    {
           printf("Error receiving message from client\n");
    }
    else
    {
		memcpy(&regn, buff, sizeof(regn));
		printf("PID:%d\n", regn.clientPid);
		printf("Cliente:%s\n", regn.cliente);
		printf("Metodo:%s\n", regn.metodo);
		printf("MG:%s\n", regn.message);
		printf("Size:%d\n", sizeof(regn));
    }

    close(sockfd);
    close(new_fd);
}

void ObtenerCurrentPath()
{
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	   fprintf(stdout, "Current working dir: %s\n", cwd);
	else
	   perror("getcwd() error");
}

void connect_to_planificador()
{
	printf("Inicia main de un socket\n");

	struct addrinfo hints, *res;
	int sockfd;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	printf("Busca info del socket\n");
	getaddrinfo("192.168.1.67", "7777", &hints, &res);
	//getaddrinfo("10.15.126.178", "7777", &hints, &res);

	printf("Obtenida info del socket\n");

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	printf("Arme la estructura del socket\n");

	connect(sockfd, res->ai_addr, res->ai_addrlen);
	printf("Me conecte\n");

	t_identificador identificador;
	identificador.id = 1;

	char* data;
	data = malloc(sizeof(identificador));

	memcpy(data, &identificador, sizeof(identificador));

	printf("Size:%d\n", sizeof(data));
	printf("Esto voy a mandar: %i\n", identificador.id);
	printf("Llego a mandar el socket\n");

	sendall(sockfd, data, sizeof(identificador));

	while(true)
	{
		printf("Trato de leer lo que me envia el planificador..\n");

		t_msg_de_planificador msg_de_planificador;
		int length = sizeof(msg_de_planificador.id_proceso) +
				sizeof(msg_de_planificador.proxima_sentencia) +
				sizeof(msg_de_planificador.quantum) +
				sizeof(msg_de_planificador.ruta_mCod_long);

		void* buffer = malloc(length);

		if(recv(sockfd, buffer, length, 0) > 0)
		{
			// Si es del tipo CPU lo acepto, si no descartamos el socket
			memcpy(&msg_de_planificador.id_proceso, buffer, sizeof(msg_de_planificador.id_proceso));
			memcpy(&msg_de_planificador.proxima_sentencia, buffer + 4, sizeof(msg_de_planificador.proxima_sentencia));
			memcpy(&msg_de_planificador.quantum, buffer + 8, sizeof(msg_de_planificador.quantum));
			memcpy(&msg_de_planificador.ruta_mCod_long, buffer + 12, sizeof(msg_de_planificador.ruta_mCod_long));

			printf("Se recibio del planificador el proceso ID: %i\n", msg_de_planificador.id_proceso);
			printf("Se recibio del planificador el la proxima sentencia: %i\n", msg_de_planificador.proxima_sentencia);
			printf("Se recibio del planificador el quantum: %i\n", msg_de_planificador.quantum);
			printf("Se recibio del planificador el largo de ruta: %i\n", msg_de_planificador.ruta_mCod_long);

			void* buff_path = malloc(msg_de_planificador.ruta_mCod_long);


			if(recv(sockfd, buff_path, msg_de_planificador.ruta_mCod_long, 0) > 0)
			{
				msg_de_planificador.ruta_mCod = malloc(msg_de_planificador.ruta_mCod_long);
				strcpy(msg_de_planificador.ruta_mCod, buff_path);
				//memcpy(&mslg_de_planificador.ruta_mCod, buff_mensaje_ruta_mCod_planificador, msg_de_planificador.ruta_mCod_long);
				printf("Se recibio del planificador la ruta: %s\n", (char *)buff_path);
			}
		}

		sleep(120);

		// Trato de enviarle un proceso
		enviar_pcb_a_planificador(sockfd, msg_de_planificador.id_proceso);
	}
}

void enviar_pcb_a_planificador(int fd, int id_proceso)
{
//	typedef struct {
//		int id_proceso;
//		int ultima_sentencia;
//		int motivo; // 0: error, 1: finalizo, 2: termino rafaga, 3: termino E/S, 4: bloqueado
//		int sleep;
//		int mensaje_long;
//		char* mensaje;
//	} t_msg_a_planificador;

	printf("Trato de enviar el PCB al planificador, fd: %i, mProc ID: %i", fd, id_proceso);

	t_msg_a_planificador* msg_a_enviar = malloc(sizeof(t_msg_a_planificador));
	msg_a_enviar->id_proceso = id_proceso;
	msg_a_enviar->ultima_sentencia = 3;
	msg_a_enviar->motivo = 2;
	msg_a_enviar->sleep = 2;
	msg_a_enviar->mensaje = "Prueba de mensaje desde la CPU";
	msg_a_enviar->mensaje_long = strlen(msg_a_enviar->mensaje) + 1;

	int length = sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + msg_a_enviar->mensaje_long;

	printf("[CPU] Envio el nuevo pcb con ID: %i\n", msg_a_enviar->id_proceso);
	printf("[CPU] Envio el nuevo pcb con ultima_sentencia: %i\n", msg_a_enviar->ultima_sentencia);
	printf("[CPU] Envio el nuevo pcb con motivo: %i\n", msg_a_enviar->motivo);
	printf("[CPU] Envio el nuevo pcb con sleep: %i\n", msg_a_enviar->sleep);
	printf("[CPU] Envio el nuevo pcb con mensaje_long: %i\n", msg_a_enviar->mensaje_long);
	printf("[CPU] Envio el nuevo pcb con mensaje: %s\n", msg_a_enviar->mensaje);
	printf("[CPU] El tamaño de lo que voy a enviar es: %i\n", length);

	void* buffer = malloc(length);
	memcpy(buffer, &(msg_a_enviar->id_proceso), sizeof(int));
	memcpy(buffer + 4, &(msg_a_enviar->ultima_sentencia), sizeof(int));
	memcpy(buffer + 8, &(msg_a_enviar->motivo), sizeof(int));
	memcpy(buffer + 12, &(msg_a_enviar->sleep), sizeof(int));
	memcpy(buffer + 16, &(msg_a_enviar->mensaje_long), sizeof(int));
	strcpy(buffer + 20, msg_a_enviar->mensaje);

	sendall(fd, buffer, length);

	free(buffer);
	free(msg_a_enviar);
}

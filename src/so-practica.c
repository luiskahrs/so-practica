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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

void PruebaMMap(){

    int fd, offset;
    char *data;
    struct stat sbuf;

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
    if (offset < 0 || offset > sbuf.st_size-1) {
        fprintf(stderr, "mmapdemo: offset must be in the range 0-%d\n", sbuf.st_size-1);
        exit(1);
    }

    data = mmap((caddr_t)0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    if (data == (caddr_t)(-1)) {
        perror("mmap");
        exit(1);
    }

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

int main(int argc, char **argv) {
	printf("Inicia main de un socket\n");
	struct addrinfo hints, *res;
	int sockfd;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	printf("Busca info del socket\n");
	getaddrinfo("192.168.1.112", "7777", &hints, &res);

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

	memcpy(buffer, "holitas", 8);

	printf("Llego a mandar el socket");
	sendall(sockfd, buffer, size);
	return 0;
}

//int main(int argc, char *argv[])
//{
//	//PruebaMMap();
//	//PruebaMongoDB();
//	PruebaSocket();
//    return 0;
//}

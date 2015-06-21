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

int main(int argc, char *argv[])
{
	//PruebaMMap();
	PruebaMongoDB();
    return 0;
}

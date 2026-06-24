#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/epoll.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include "render.h"
#include "parser.h"

#define MAX_PATH_LENGTH 4096



char* leerArchivo(const char* path, long *size)
{

        FILE* file = fopen(path, "rb"); if(!file) return NULL;
        if(fseek(file,0, SEEK_END)!=0) goto error;
        *size = ftell(file); if(*size <0) goto error;
        if(fseek(file,0, SEEK_SET)!=0) goto error;
        char* buffer = malloc (*size+1); if(!buffer) goto error;
        if(fread(buffer, 1, *size, file)!= *size){free(buffer); goto error;}
        fclose(file);
        buffer[*size] = '\0';
        return buffer;
error:
        fclose(file);
        return NULL;
}

void parsear(char* file, long fileSize,modelo_t* modelo){
    char* actual = (char*)file;
    while(*actual){
        if (*actual == 'v' && actual[1] == ' ') { modelo->cantidadVertices ++;}
        else if (*actual == 'f' && actual[1] == ' ') {modelo->cantidadPoligonos ++;}
        while (*actual && *actual != '\n') actual++;
        if (*actual == '\n')actual++;
    }
    actual = (char*)file;
    modelo->poligonos = malloc(sizeof(int[3])*modelo->cantidadPoligonos);
    modelo->vertices = malloc(sizeof(punto3D_t)*modelo->cantidadVertices);
    size_t itPoligonos = 0;
    size_t itVertices = 0;

    int cantLineas = 0; //esto es para el print nada mas

    while (*actual) {
        if (*actual == 'v' && actual[1] == ' '){
            punto3D_t* punto = &modelo->vertices[itVertices++];
            sscanf(actual,"v %f %f %f",&punto->x,&punto->y,&punto->z);
        }

        else if (*actual == 'f' && actual[1] == ' '){
            int v1, vt1, vn1, v2, vt2, vn2, v3, vt3, vn3, v4;
    
            if(sscanf(actual, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3) == 9){
                modelo->poligonos[itPoligonos][0] = v1 - 1;
                modelo->poligonos[itPoligonos][1] = v2 - 1;
                modelo->poligonos[itPoligonos][2] = v3 - 1;
                itPoligonos++;
            }
            else if(sscanf(actual, "f %d//%d %d//%d %d//%d", &v1, &vn1, &v2, &vn2, &v3, &vn3) == 6){
                modelo->poligonos[itPoligonos][0] = v1 - 1;
                modelo->poligonos[itPoligonos][1] = v2 - 1;
                modelo->poligonos[itPoligonos][2] = v3 - 1;
                itPoligonos++;
            }
            /* quads no estan soportados ya que deberia de incrementar por 2 el numero de itPoligonos por quad antes de hacer el malloc
            else if(sscanf(actual, "f %d %d %d %d", &v1, &v2, &v3, &v4) == 4){ //supongo que mantiene la normal
                modelo->poligonos[itPoligonos][0] = v1 - 1;
                modelo->poligonos[itPoligonos][1] = v2 - 1;
                modelo->poligonos[itPoligonos][2] = v3 - 1;
                itPoligonos++;

                modelo->poligonos[itPoligonos][0] = v1 - 1;
                modelo->poligonos[itPoligonos][1] = v3 - 1;
                modelo->poligonos[itPoligonos][2] = v4 - 1;
                itPoligonos++;
            }
                */
            else if(sscanf(actual, "f %d %d %d", &v1, &v2, &v3) == 3){
                modelo->poligonos[itPoligonos][0] = v1 - 1;
                modelo->poligonos[itPoligonos][1] = v2 - 1;
                modelo->poligonos[itPoligonos][2] = v3 - 1;
                itPoligonos++;
            }            
        }

        while (*actual && *actual != '\n'){actual++;} 
        if (*actual == '\n'){printf("%d\n",cantLineas++);actual++;} 
    }
    free(file);

}

int parsearModelos(modelo_t** modelos,size_t* cantidadModelos){
        char path[MAX_PATH_LENGTH];
        if (realpath("./modelos", path) == NULL) return -1;
        size_t lengthPath = strlen(path);
        
        struct stat info;
        if(stat(path, &info)!=0){return -1;}        

        if (S_ISDIR(info.st_mode)) {
            DIR* dir = opendir(path);if (dir == NULL){return -1;}
            struct dirent *entrada;
            
            *cantidadModelos = 0;
            while ((entrada = readdir(dir)) != NULL) {
                if (strcmp(entrada->d_name, ".") == 0 ||strcmp(entrada->d_name, "..") == 0) continue;
                (*cantidadModelos)++;
            }

            *modelos = malloc(sizeof(modelo_t) * (*cantidadModelos));
            size_t itModelos = 0;
            rewinddir(dir);

            while ((entrada = readdir(dir)) != NULL) {

                if (strcmp(entrada->d_name, ".") == 0 ||strcmp(entrada->d_name, "..") == 0){continue;}

                char rutaCompleta[MAX_PATH_LENGTH];
                snprintf(rutaCompleta,sizeof(rutaCompleta),"%s/%s",path,entrada->d_name);
                
                long fileSize;
                char* file = leerArchivo(rutaCompleta, &fileSize);if (!file)continue;
                
                modelo_t modelo = {0};

                parsear(file, fileSize,&modelo);

                (*modelos)[itModelos++] = modelo;
            }

        closedir(dir);
    }
    return 0;
}
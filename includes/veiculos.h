#ifndef _VEICULOS_H_
#define _VEICULOS_H_

#define STATIC_VEHICLE_SIZE 31

typedef struct{
    char removido;          // Status do veiculo
    int tamanhoRegistro;    // Numero de bytes do registro
    char prefixo[6];        // Prefixo do veiculo
    char data[10];          // Data de entrada do veiculo na frota
    int quantidadeLugares;  // Quantidade de lugares sentados disponiveis
    int codLinha;           // Linha associada ao veiculo
    int tamanhoModelo;      // Numero de bytes do campo modelo
    char *modelo;           // Modelo do veiculo
    int tamanhoCategoria;   // Numero de bytes do campo categoria
    char *categoria;        // Categoria do veiculo
} Veiculo;

void funcionalidade1();
void funcionalidade3();
void funcionalidade5();
void funcionalidade7();
void funcionalidade9();
void funcionalidade11();
void funcionalidade13();
void funcionalidade15();
int funcionalidade17(char *veiculoDesordenadoNome, char* veiculoOrdenadoNome, char *campo);
void funcionalidade19();
int compareVeiculos(const void *a, const void *b);
void escreveBinVeiculos(FILE *binPointer, Veiculo *v);
char *converteData(char *data);
void printVeiculoMatch(Veiculo *vehicle);

#endif
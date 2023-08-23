#ifndef _FILE_H_
#define _FILE_H_

#define readlineBuffer 64

typedef struct{
    char status;
    long long byteProxReg;
    int nroRegistros;
    int nroRegRemovidos;
} Binary;

void binarioNaTela(char *nomeArquivoBinario);
void scan_quote_string(char *str);
int convertePrefixo(char* str);
Binary *leCabecalho(FILE *binPointer);
char *freadline(FILE *stream, char *line);
void escreveCabecalhoVeiculos(FILE *binPointer, Binary *b);
void escreveCabecalhoLinhas(FILE *binPointer, Binary *bin);
void copiaCabecalhoLinhas(FILE *csvPointer, FILE *binPointer, Binary *bin);
void salvaCabecalho(FILE *binPointer, char **cab1, char **cab2, char **cab3, char **cab4, char **cab5);

#endif
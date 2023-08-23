#ifndef __LINHAS_H__
#define __LINHAS_H__

typedef struct{
    char removido;
    int tamanhoRegistro;
    int codLinha;
    char aceitaCartao;
    int tamanhoNome;
    char *nomeLinha;
    int tamanhoCor;
    char *corLinha;
} Linha;

void funcionalidade2();
void funcionalidade4();
void funcionalidade6();
void funcionalidade8();
void funcionalidade10();
void funcionalidade12();
void funcionalidade14();
void funcionalidade16();
int funcionalidade18(char *linhaDesordenadaNome, char *linhaOrdenadaNome, char *campo);
int compareLinhas(const void *a, const void *b);
Linha *leLinha(FILE *fp, long long Pr);
void escreveBinLinhas(FILE *fp, Linha *line);
char *cartao(char type);
void leLinhaEImprime(FILE *fp, long long Pr);
void printBusLine(FILE *fp, Linha *line);
int findRegister(Linha *line, char *nomeDoCampo, char *valor);
void printLinhaMatch(Linha *line);

#endif
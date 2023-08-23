#ifndef _ARVORE_B_H_
#define _ARVORE_B_H_

#define LEN_TREE_PAGE 77
#define ORDEM 5
#define MIN 2
#define PROMOTION 1
#define NO_PROMOTION 0
#define NOT_FOUND -1
#define ADD 1
#define SEARCH 0

typedef struct page{
    int C[ORDEM];
    int P[ORDEM + 1];
    int nroChavesIndexadas;
    int RRNdoNo;
    long long Pr[ORDEM];
    char folha;
}Page;

typedef struct btree{
    char status;
    int noRaiz;
    int RRNproxNo;
    Page *root;
    FILE *arvorePointer;
}Btree;

Btree *loadBTree(char *arvore, int flag);
long long buscaBinaria(Page *pagina, FILE *fp, int chave, long long RRN);
Btree *createBTree(char *arvore, int C, long Pr);
Page *createRoot(int C, long Pr);
Page *createPage(int C, int Pr, int root, int child, int RRN);
Page *createEmptyPage(int RRN);
void setStatus(Btree **tree, char status);
char getStatus(Btree *tree);
void setNoRaiz(Btree **tree, int noRaiz);
int getNoRaiz(Btree *tree);
void setRRNproxNo(Btree **tree, int RRNproxNo);
int getRRNproxNo(Btree *tree);
void setRoot(Btree **tree, Page *root);
Page *getRoot(Btree *tree);
void setArvorePointer(Btree **tree, FILE *arvorePointer);
FILE *getArvorePointer(Btree *tree);
Page *copyPage(FILE *arvorePointer, int RRN);
void escreveCabecalhoArvoreB(Btree *tree);
void escreveBinArvoreB(Page *page, FILE *arvorePointer, int RRN);
void insertion(Btree **tree, int C, long Pr);
int insert(Btree *tree, int C, int *promo_C, long Pr, long *promo_Pr, int RRN, int *child);
void insertValue(Btree *tree, int C, long Pr, int pos, int RRN, int P);
void splitNode(Btree *tree, int C, int *promo_C, long Pr, long *promo_Pr, int pos, int *child, Page *page, Page **newPage);
void imprime_bin(FILE *arvorePointer);
void ftree(Btree *tree);
void froot(Btree *tree);
long long search(Btree *tree, int C, int *pos, int RRN);

#endif
#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "arvore-b.h"

//================================As funções a seguir servem para criar novas árvores ou páginas:================================
// Essa função cria uma árvore em seu estado inicial, com apenas uma página criada
Btree *createBTree(char *arvore, int C, long Pr){
    Btree *tree = (Btree *)malloc(sizeof(Btree)); 
    tree->root = createRoot(C, Pr);

    tree->arvorePointer = fopen(arvore, "wb+");   // Abre em modo de criação e escrita de arquivo binário
    tree->status = '0';
    tree->noRaiz = 0;
    tree->RRNproxNo = 1;

    return tree;
}

// Essa função copia uma árvore de um arquivo anterior
Btree *loadBTree(char *arvore, int flag){
    Btree *tree = (Btree *)malloc(sizeof(Btree)); 

    if(flag == ADD) tree->arvorePointer = fopen(arvore, "rb+");   // Abre em modo de leitura e escrita de arquivo binário
    if(flag == SEARCH) tree->arvorePointer = fopen(arvore, "rb");   // Abre em modo de leitura de arquivo binário

    if(tree->arvorePointer == NULL){
        free(tree);
        return NULL;
    }

    fseek(tree->arvorePointer, 0L, SEEK_SET);
    
    fread(&tree->status, sizeof(char), 1, tree->arvorePointer);
    fread(&tree->noRaiz, sizeof(int), 1, tree->arvorePointer);
    fread(&tree->RRNproxNo, sizeof(int), 1, tree->arvorePointer);

    return tree;
}

// Essa função cria a primeira raiz adiciona a primeira chave
Page *createRoot(int C, long Pr){
    Page *newPage = (Page *)malloc(sizeof(Page));
    int i;

    newPage->folha = '1';
    newPage->RRNdoNo = 0;
    newPage->nroChavesIndexadas = 1;

    newPage->C[0] = C;
    newPage->Pr[0] = Pr;
    newPage->P[0] = -1;
    for(i = 1; i < ORDEM; i++){
        newPage->C[i] = -1;
        newPage->Pr[i] = -1;
        newPage->P[i] = -1;
    }
    newPage->P[ORDEM] = -1; 

    return newPage;
}

// essa função cria uma nova raiz quando a função split promove um valor para acima do nó raiz atual
Page *createPage(int C, int Pr, int root, int child, int RRN){
    Page *newPage = (Page *)malloc(sizeof(Page));
    int i;
    
    newPage->folha = '0';
    newPage->RRNdoNo = RRN;
    newPage->nroChavesIndexadas = 1;
    
    newPage->C[0] = C;
    newPage->Pr[0] = Pr;
    for(i = 1; i < ORDEM; i++){
        newPage->C[i] = -1;
        newPage->Pr[i] = -1;
    }

    newPage->P[0] = root;
    newPage->P[1] = child;
    for(i = 2; i < ORDEM + 1; i++) newPage->P[i] = -1;

    return newPage;
}

// Essa função cria uma nova página completamente vazia com o RRN desejado
Page *createEmptyPage(int RRN){
    Page *newPage = (Page *)malloc(sizeof(Page));
    int i;
    
    newPage->folha = '1';
    newPage->RRNdoNo = RRN;
    newPage->nroChavesIndexadas = 0;
    for(i = 0; i < ORDEM; i++){
        newPage->C[i] = -1;
        newPage->Pr[i] = -1;
        newPage->P[i] = -1;
    }
    newPage->P[ORDEM] = -1;

    return newPage;
}

//=============================Aqui estão as funções que mexem diretamente com o arquivo da Árvore-B:=============================
// Essa função copia a página do RRN solicitado para uma page da B-Tree para que possa ser manipulada
Page *copyPage(FILE *arvorePointer, int RRN){
    Page *newPage = (Page *)malloc(sizeof(Page));
    int i;
    
    fseek(arvorePointer, ((RRN + 1) * LEN_TREE_PAGE), SEEK_SET);
    
    fread(&newPage->folha, sizeof(char), 1, arvorePointer);
    fread(&newPage->nroChavesIndexadas, sizeof(int), 1, arvorePointer);
    fread(&newPage->RRNdoNo, sizeof(int), 1, arvorePointer);
    
    for(i = 0; i < ORDEM - 1; i++){
        fread(&newPage->P[i], sizeof(int), 1, arvorePointer);
        fread(&newPage->C[i], sizeof(int), 1, arvorePointer);
        fread(&newPage->Pr[i], sizeof(long), 1, arvorePointer);
    }
    fread(&newPage->P[ORDEM - 1], sizeof(int), 1, arvorePointer);

    return newPage;    
}    

// Essa função escreve os valores requisitados no cabeçalho da B-Tree
void escreveCabecalhoArvoreB(Btree *tree){
	fseek(tree->arvorePointer, 0L, SEEK_SET);	// Vai para o começo do arquivo (cabeçalho)

    fwrite(&tree->status, sizeof(char), 1, tree->arvorePointer);
    fwrite(&tree->noRaiz, sizeof(int), 1, tree->arvorePointer);
    fwrite(&tree->RRNproxNo, sizeof(int), 1, tree->arvorePointer);
	for(int i = 0; i < 68; i++) fwrite("@", sizeof(char), 1, tree->arvorePointer);
}

// Essa função escreve a página solicitada no RRN solicitado
void escreveBinArvoreB(Page *page, FILE *arvorePointer, int RRN){
    fseek(arvorePointer, ((RRN + 1) * LEN_TREE_PAGE), SEEK_SET);
    
    fwrite(&page->folha, sizeof(char), 1, arvorePointer);
    fwrite(&page->nroChavesIndexadas, sizeof(int), 1, arvorePointer);
    fwrite(&page->RRNdoNo, sizeof(int), 1, arvorePointer);

    for(int i = 0; i < ORDEM - 1; i++){
        fwrite(&page->P[i], sizeof(int), 1, arvorePointer);
        fwrite(&page->C[i], sizeof(int), 1, arvorePointer);
        fwrite(&page->Pr[i], sizeof(long), 1, arvorePointer);
    }
    fwrite(&page->P[ORDEM - 1], sizeof(int), 1, arvorePointer);
}

//===========================Aqui estão as funções relacionadas com o processo de inserção na Árvore-B:===========================
void insertion(Btree **tree, int C, long Pr){
    // Cria as chaves que serão usadas para salvar as chaves promovidas
    int promo_C, child;
    long promo_Pr;

    // Chama a função insert que busca onde inserir a chave
    if(insert(*tree, C, &promo_C, Pr, &promo_Pr, (*tree)->noRaiz, &child) == PROMOTION){
        // Caso uma chave deva ser promovida para além da raiz, uma nova raiz é criada com split
        (*tree)->root = createPage(promo_C, promo_Pr, (*tree)->noRaiz, child, (*tree)->RRNproxNo);
        (*tree)->noRaiz = (*tree)->RRNproxNo;
        (*tree)->RRNproxNo++;

        // Escreve essa nova raiz no arquivo binário
        escreveBinArvoreB((*tree)->root, (*tree)->arvorePointer, (*tree)->noRaiz);
        free((*tree)->root);
    }
}

int insert(Btree *tree, int C, int *promo_C, long Pr, long *promo_Pr, int RRN, int *child){
    // Caso chegue em uma página vazia, quer dizer que encontrou onde deve adicionar a chave
    if(RRN == -1){
        *promo_C = C;
        *promo_Pr = Pr;
        *child = -1;
        
        return PROMOTION;
    }

    int pos;
    Page *page = copyPage(tree->arvorePointer, RRN), *newPage;
    
    // Checa para o caso de uma folha não possuir mais essa denominação
    if(page->folha == '1'){
        for(int i = 0; i < ORDEM; i++){
            if(page->P[i] != -1){
                page->folha = '0';
                escreveBinArvoreB(page, tree->arvorePointer, RRN);
            }
        }
    }

    // Procura onde inserir a nova chave
    if(C < page->C[0]) pos = 0;
    else{
        for(pos = page->nroChavesIndexadas; (C < page->C[pos - 1] && pos > 1); pos--);
    }

    // chama recurvivamente o processo de inserção
    if(insert(tree, C, &(*promo_C), Pr, &(*promo_Pr), page->P[pos], &(*child)) == PROMOTION){
        // Caso ocorra uma promoção, confere se há espaço para a inserção de uma nova chave
        if(page->nroChavesIndexadas < (ORDEM - 1)){
            insertValue(tree, *promo_C, *promo_Pr, pos, RRN, *child);
        }else{  // Caso contrário, realiza um split
            splitNode(tree, *promo_C, promo_C, *promo_Pr, promo_Pr, pos, &(*child), page, &newPage);
            return PROMOTION;
        }
    }

    free(page);
    return NO_PROMOTION;
}

void insertValue(Btree *tree, int C, long Pr, int pos, int RRN, int P){
    Page *page = copyPage(tree->arvorePointer, RRN);
    int j;

    // Passa todas as chaves após a posição da chave a ser inserida para a próxima
    for(j = page->nroChavesIndexadas; j > pos; j--){
        page->C[j] = page->C[j - 1];
        page->Pr[j] = page->Pr[j - 1];
        page->P[j + 1] = page->P[j];
    }

    // Insere a chave na posição desejada
    page->C[j] = C;
    page->Pr[j] = Pr;
    page->P[j + 1] = P;
    page->nroChavesIndexadas++; // Incrementa o número de chaves na página

    escreveBinArvoreB(page, tree->arvorePointer, RRN);  // Escreve (ou sobrescreve) a página no arquivo
    free(page);
}

void splitNode(Btree *tree, int C, int *promo_C, long Pr, long *promo_Pr, int pos, int *P, Page *page, Page **newPage){
    int j;

    // Passa todas as chaves após a posição da chave a ser inserida para a próxima, fazendo a página ter 5 chaves
    for(j = 4; j > pos; j--){
        page->C[j] = page->C[j - 1];
        page->Pr[j] = page->Pr[j - 1];
        page->P[j + 1] = page->P[j];
    }
    page->C[j] = C;
    page->Pr[j] = Pr;
    page->P[j + 1] = *P;

    // Cria um novo nó com o RRN desejado e divide a page em duas
    *newPage = createEmptyPage(tree->RRNproxNo++);
    (*newPage)->C[0] = page->C[3]; (*newPage)->Pr[0] = page->Pr[3]; (*newPage)->P[0] = page->P[3];
    (*newPage)->C[1] = page->C[4]; (*newPage)->Pr[1] = page->Pr[4]; (*newPage)->P[1] = page->P[4];
    (*newPage)->P[2] = page->P[5];
    (*newPage)->nroChavesIndexadas = 2; // Atualiza o número de chaves de newPage

    // Salva os valores que serão promovidos
    *promo_C = page->C[2];
    *promo_Pr = page->Pr[2];

    // Exclui da page, os valores promovidos e os da newPage
    page->C[2] = page->C[3] = page->C[4] = -1;
    page->Pr[2] = page->Pr[3] = page->Pr[4] = -1;
    page->P[3] = page->P[4] = page->P[5] = -1;
    page->nroChavesIndexadas = 2; // Atualiza o número de chaves de page

    // Confere se page é ainda um nó folha
    if(page->folha == '1'){
        for(int i = 0; i < ORDEM; i++){
            if(page->P[i] != -1){
                page->folha = '0';
            }
        }
    }
    escreveBinArvoreB(page, tree->arvorePointer, page->RRNdoNo);
    
    // Confere se newPage é uma folha
    if((*newPage)->folha == '1'){
        for(int i = 0; i < ORDEM; i++){
            if((*newPage)->P[i] != -1){
                (*newPage)->folha = '0';
            }
        }
    }
    escreveBinArvoreB((*newPage), tree->arvorePointer, (*newPage)->RRNdoNo);

    // Salva o valor de P a ser promovido como o RRN da newPage
    *P = (*newPage)->RRNdoNo;
    free(page);
    free(*newPage);
}

//======================================Aqui está a função de busca no arquivo da Árvore-B:======================================
long long search(Btree *tree, int C, int *pos, int RRN){
    // Caso chegue à uma página nula, quer dizer que não há o registro solicitado na Árvore-B
    if(RRN == -1) {
      return NOT_FOUND;
    }

    Page *page = copyPage(tree->arvorePointer, RRN);

    // Procura a posição em que a chave deveria estar
    if(C < page->C[0]) *pos = 0;
    else{
        for(*pos = page->nroChavesIndexadas - 1; (C < page->C[*pos] && *pos > 0); (*pos)--);
        // Caso seja encontrada, retorna o byte onde está o veículo no veiculo.bin
        if(C == page->C[*pos]){
            long Pr = page->Pr[*pos];
            free(page);
            return Pr; 
        }
    }

    RRN = page->P[*pos + 1];
    free(page);

    // Caso não tenha encontrado, continua a busca recursivamente
    return search(tree, C, &(*pos), RRN);
}

//===================================Aqui estão as funções de desalocação dinâmica da Árvore-B:===================================
void ftree(Btree *tree){
    fclose(tree->arvorePointer);
    free(tree);
}

void froot(Btree *tree){
    free(tree->root);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <linhas.h>
#include <veiculos.h>
#include <file.h>
#include <arvore-b.h>

#define STATIC_LINE_SIZE 13 //tamanho dos dados de tamanho fixo das linhas (removido, tamanhoRegistro, codLinha, aceitaCartao e tamanhoNome)

void funcionalidade2(){
    FILE *csvPointer; //para essa funcao, o cabecalho e os dados serão convertidos e escritos para o binário
    fgetc(stdin);

    char linhacsv[30];
    scanf("%s", linhacsv);

    csvPointer = fopen(linhacsv, "r");

    if(csvPointer == NULL){ //caso de csv inexistente
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    fgetc(stdin);

    FILE *binPointer;
    char linhabin[30];

    scanf("%s", linhabin);
    binPointer = fopen(linhabin, "wb+");

    //a struct bin salvará os dados do cabeçálho do csv para colocá-los no arquivo bin
    
    //a struct line será usada para armazenar os dados das linhas, será alocada uma vez e desalocada no final, sendo reutilizada
    //diversas vezes para economizar espaço, essa característica foi usada em todas as funções deste arquivo (2, 4, 6, 8)
    Binary *bin = (Binary*)malloc(sizeof(Binary));
    Linha *line = (Linha*)malloc(sizeof(Linha));

    //os campos nomeLinha e corLinha serão alocados aqui e desalocados no fim, com um limite de caracteres fixo, pois nenhuma linha tem nome ou cor maiores
    line->nomeLinha = (char*)malloc(sizeof(char)*64);
    line->corLinha = (char*)malloc(sizeof(char)*16);

    bin->byteProxReg = 83;
    bin->nroRegistros = 0;
    bin->nroRegRemovidos = 0;
    bin->status = '0'; //status de arquivo inconsistente
    
    copiaCabecalhoLinhas(csvPointer, binPointer, bin); //transfere o cabeçalho do csv para o bin

    char letter;
    while(fread(&letter, sizeof(char), 1, csvPointer) != 0){    // Armazena em letter a primeira letra da linha para conferir se o cursor está no final
        if(letter == '*'){      // Aproveitando que letter armazena a primera letra, dá para saber se o arquivo deve ser removido
            line->removido = '0';
            bin->nroRegRemovidos++;
        }else{
            line->removido = '1';
            fseek(csvPointer, -1L, SEEK_CUR);
            bin->nroRegistros++;
        }
        //aqui serão lidos cada um dos campos, e no fim, escritos para o arquivo .bin
        fscanf(csvPointer, "%d", &line->codLinha);
        fgetc(csvPointer);  // Descarta a vírgula entre as categorias

        fscanf(csvPointer, "%c", &line->aceitaCartao);

        fgetc(csvPointer);

        freadline(csvPointer, line->nomeLinha);
        line->tamanhoNome = strlen(line->nomeLinha);
        
        freadline(csvPointer, line->corLinha);
        line->tamanhoCor = strlen(line->corLinha);

        //o tamanho do registro contém o tamanho dos campos estáticos e dos dinâmicos
        line->tamanhoRegistro = STATIC_LINE_SIZE + line->tamanhoNome + line->tamanhoCor;

        //escrita do arquivo bin
        escreveBinLinhas(binPointer, line);
    }
    
    bin->byteProxReg = ftell(binPointer);
    bin->status = '1'; //aqui o arquivo bin está finalmente consistente

    escreveCabecalhoLinhas(binPointer, bin); //reescrita do cabeçalho e finalização da função
    free(bin);
    free(line->nomeLinha);
    free(line->corLinha);
    free(line);

    fclose(binPointer);
    fclose(csvPointer);

    binarioNaTela(linhabin);
}

void funcionalidade4(){
    fgetc(stdin);
    char bin[30];
    scanf("%s", bin);
    FILE *fp = fopen(bin, "rb");
    
    if(fp == NULL){
        printf("Registro inexistente.\n");
        return;
    }

    char letter;
    
    fread(&letter, sizeof(char), 1, fp);

    if(letter == '0'){
        printf("Falha no processamento do arquivo.\n");
        return;
    }


    fseek(fp, 82L, SEEK_SET);

    if(fread(&letter, sizeof(char), 1, fp) == 0){
        printf("Registro inexistente.\n");
        fclose(fp);
        return;
    }
    Linha *line = (Linha*)malloc(sizeof(Linha));
    line->nomeLinha = (char*)malloc(sizeof(char)*64);
    line->corLinha = (char*)malloc(sizeof(char)*64);

    fseek(fp, -1L, SEEK_CUR);
    while(fread(&letter, sizeof(char), 1, fp) != 0){
        fread(&line->tamanhoRegistro, sizeof(int), 1, fp);

        if(letter == '0') fseek(fp, line->tamanhoRegistro, SEEK_CUR);
        else{
            //caso a linha não seja removida, será lida e printada ao fim desse condicional else
            fread(&line->codLinha, sizeof(int), 1, fp);
            fread(&line->aceitaCartao, sizeof(char), 1, fp);
            fread(&line->tamanhoNome, sizeof(int), 1, fp);

            if(line->tamanhoNome != 0)fread(line->nomeLinha, line->tamanhoNome, 1, fp);
            else line->nomeLinha[0] = '\0';

            fread(&line->tamanhoCor, sizeof(int), 1, fp);
            fread(line->corLinha, line->tamanhoCor, 1, fp);
            printBusLine(fp, line);
        }

    }

    free(line->nomeLinha);
    free(line->corLinha);
    free(line);
    fclose(fp);
}

void funcionalidade6(){
    char bin[30];
    scanf("%s", bin);

    char nomeDoCampo[32];
    scanf("%s", nomeDoCampo);

    char valor[64];
    scan_quote_string(valor);

    FILE *fp = fopen(bin, "rb");

    //testes de arquivo inexistente ou inconsistente
    if(bin == NULL){
        printf("Registro inexistente.\n");
        return;
    }
    char letter;

    fread(&letter, sizeof(char), 1, fp);
    if(letter == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(fp);
        return;
    }
    Linha *line = (Linha*)malloc(sizeof(Linha));

    line->nomeLinha = (char*)malloc(sizeof(char)*128);
    line->corLinha = (char*)malloc(sizeof(char)*128);

    fseek(fp, 82L, SEEK_SET);

    while(fread(&letter, sizeof(char), 1, fp) != 0){
        fread(&line->tamanhoRegistro, sizeof(int), 1, fp);

        if(letter == '0') fseek(fp, line->tamanhoRegistro, SEEK_CUR);
        else{
            //aqui as linhas serão lidas e os matches serão buscados pela funcao findRegister, caso seja um match, a linha será printada
            fread(&line->codLinha, sizeof(int), 1, fp);
            fread(&line->aceitaCartao, sizeof(char), 1, fp);
            fread(&line->tamanhoNome, sizeof(int), 1, fp);

            if(line->tamanhoNome != 0)fread(line->nomeLinha, line->tamanhoNome, 1, fp);
            else line->nomeLinha[0] = '\0';

            fread(&line->tamanhoCor, sizeof(int), 1, fp);
            fread(line->corLinha, line->tamanhoCor, 1, fp);
            line->corLinha[line->tamanhoCor] = '\0';
            
            if(findRegister(line, nomeDoCampo, valor)) printBusLine(fp, line);
        }

    }

    free(line->nomeLinha);
    free(line->corLinha);

    free(line);
    fclose(fp);
}

void funcionalidade8(){
    char bin[32];
    scanf("%s", bin);

    int number;
    scanf("%d", &number);

    FILE *fp = fopen(bin, "rb+");

    //testes de arquivo inexistente ou inconsistente
    if(fp == NULL){
        printf("Registro inexistente.\n");
        return;
    }

    char fileStatus;

    fread(&fileStatus, sizeof(char), 1, fp);
    if(fileStatus == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(fp);
        return;
    }
    Binary *binHeader = (Binary*)malloc(sizeof(Binary));
    Linha *line = (Linha*)malloc(sizeof(Linha));
    line->nomeLinha = (char*)malloc(sizeof(char)*64);
    line->corLinha = (char*)malloc(sizeof(char)*64);

    binHeader->status = '0'; //marcando inconsistência do arquivo e escrevendo-a no cabeçalho
    fseek(fp, 0L, SEEK_SET);
    fwrite(&binHeader->status, sizeof(char), 1, fp);

    //leitura do cabeçalho bin
    fread(&binHeader->byteProxReg, sizeof(long long int), 1, fp);
    fread(&binHeader->nroRegistros, sizeof(int), 1, fp);
    fread(&binHeader->nroRegRemovidos, sizeof(int), 1, fp);

    binHeader->nroRegistros = binHeader->nroRegistros + number; //atualização do número total de registros
    fseek(fp, 0, SEEK_END);

    while(number--){
        //leitura dos dados a serem inseridos
        scanf("%d", &line->codLinha);
        scan_quote_string(&line->aceitaCartao);
        scan_quote_string(line->nomeLinha);
        scan_quote_string(line->corLinha);

        
        line->tamanhoNome = strlen(line->nomeLinha);
        line->tamanhoCor = strlen(line->corLinha);
        
        line->tamanhoRegistro = STATIC_LINE_SIZE + line->tamanhoNome + line->tamanhoCor;

        line->removido = '1';
        escreveBinLinhas(fp, line);
    }

    binHeader->byteProxReg = ftell(fp);
    
    binHeader->status = '1'; //atribuindo consistência ao bin

    //atualizando a consistência do arquivo bin e os dados byteProxReg e nroRegistros
    fseek(fp, 0L, SEEK_SET);
    fwrite(&binHeader->status, sizeof(char), 1, fp);
    fwrite(&binHeader->byteProxReg, sizeof(long long int), 1, fp);
    fwrite(&binHeader->nroRegistros, sizeof(int), 1, fp);

    free(binHeader);
    free(line->nomeLinha);
    free(line->corLinha);
    free(line);
    fclose(fp);
    binarioNaTela(bin);
}

void funcionalidade10(){
    int C, jump, codLinha;
    long long Pr;
    char removido, linha[32], arvore[32];
    FILE *linhaPointer;
    Btree *tree;

    scanf("%s", linha);
    linhaPointer = fopen(linha, "rb");  // Abre o arquivo em modo de apenas leitura

    scanf("%s", arvore);

    if(linhaPointer == NULL){   // Caso o arquivo linha.bin não exista
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Caso o arquivo esteja inconsistente, retorna com erro
    fread(&removido, sizeof(char), 1, linhaPointer);
    if(removido == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(linhaPointer);
        return;
    }

    fseek(linhaPointer, 82L, SEEK_SET);  // Pula o cabeçalho do linha.bin
    while(fread(&removido, sizeof(char), 1, linhaPointer) == '0'){    // Armazena em removido a primeira letra da linha para conferir se o cursor está no final
        fread(&jump, sizeof(int), 1, linhaPointer); // Salva o tamanho do registro, para conseguir pulá-lo
        fseek(linhaPointer, jump, SEEK_CUR);
    }
    
    fseek(linhaPointer, -1L, SEEK_CUR); // Retorna um byte para pegar o Pr correto
    Pr = ftell(linhaPointer);
            
    fseek(linhaPointer, 1L, SEEK_CUR);
    fread(&jump, sizeof(int), 1, linhaPointer); // Salva o tamanho do registro para pulá-lo

    fread(&codLinha, sizeof(int), 1, linhaPointer);

    C = codLinha;

    tree = createBTree(arvore, C, Pr);    // Struct usada para salvar as informações da Árvore-B binario
    escreveCabecalhoArvoreB(tree); // Função que escreve o cabeçalho no arquivo da Árvore-B
    escreveBinArvoreB(tree->root, tree->arvorePointer, 0);  // Função que escreve uma página no arquivo da Árvore-B
    froot(tree);
    
    fseek(linhaPointer, (jump - 4), SEEK_CUR);
    while(fread(&removido, sizeof(char), 1, linhaPointer) != 0){    // Armazena em removido a primeira letra da linha para conferir se o cursor está no final
        if(removido == '1'){      // Aproveitando que 'removido' armazena a primera letra, dá para saber se a página está logicamente removida
            fseek(linhaPointer, -1L, SEEK_CUR);   // Retona um byte para pegar o Pr correto
            Pr = ftell(linhaPointer);
            
            fseek(linhaPointer, 1L, SEEK_CUR);
            fread(&jump, sizeof(int), 1, linhaPointer); // Salva o tamanho do registro para pulá-lo

            fread(&codLinha, sizeof(int), 1, linhaPointer);
            C = codLinha;

            insertion(&tree, C, Pr); 

            fseek(linhaPointer, (jump - 4), SEEK_CUR);
        }else{
            fread(&jump, sizeof(int), 1, linhaPointer);
            fseek(linhaPointer, jump, SEEK_CUR);
        }
    }
    tree->status = '1';
    escreveCabecalhoArvoreB(tree);   // Atualiza o cabeçalho no final do programa
    ftree(tree);
    fclose(linhaPointer);

    binarioNaTela(arvore);
}

void funcionalidade12(){
    FILE *linhaPointer = NULL, *indicePointer = NULL;
    char linha[32], indice[32], codLinha[10];
    int valor;

    scanf("%s", linha);
    scanf("%s", indice);
    scanf("%s", codLinha);
    scanf("%d", &valor);

    linhaPointer = fopen(linha, "rb");
    indicePointer = fopen(indice, "rb");

    //testes de falha de processamento
    if((linhaPointer == NULL) | (indicePointer == NULL)){
        printf("Falha no processamento do arquivo.\n");
        fclose(linhaPointer);
        return;
    }

    char letter;
    fread(&letter, sizeof(char), 1, linhaPointer);
    if(letter == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(linhaPointer);
        return;
    }

    fread(&letter, sizeof(char), 1, indicePointer);
    if(letter == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(indicePointer);
        return;
    }

    int noRaiz;
    fread(&noRaiz, sizeof(int), 1, indicePointer);

    fseek(indicePointer, noRaiz, SEEK_SET);
    Page *pagina = (Page*)malloc(sizeof(Page));
    long long RRN = buscaBinaria(pagina, indicePointer, valor, noRaiz);

    leLinhaEImprime(linhaPointer, RRN);

    free(pagina);
    fclose(indicePointer);
    fclose(linhaPointer);
}

void funcionalidade14(){
    int n;
    char removido, linhaBin[32], arvore[32];
    FILE *linhaPointer;
    Binary *b = (Binary *)malloc(sizeof(Binary));   // Struct usada para salvar as informações do arq. binario

    scanf("%s", linhaBin);
    linhaPointer = fopen(linhaBin, "rb+");  // Abre o arquivo em modo de leitura e escrita binaria
    
    // Verifica se o arquivo existe
    if(linhaPointer == NULL){
        printf("Falha no processamento do arquivo.\n");
        free(b);
        return;
    }

    scanf("%s", arvore);
    Btree *tree = loadBTree(arvore, ADD);
    
    // Verifica se o arquivo indice existe
    if(tree == NULL){
        printf("Falha no processamento do arquivo.\n");
        free(b);
        fclose(linhaPointer);
        return;
    }

    scanf("%d", &n);

    // Verifica se os arquivo estão com o status consistente
    fread(&removido, sizeof(char), 1, linhaPointer);
    if(removido == '0' || tree->status == '0'){
        printf("Falha no processamento do arquivo.\n");
        free(b);
        fclose(linhaPointer);
        ftree(tree);
        return;
    }

    // Atualiza o status da árvore para modificá-la
    fseek(tree->arvorePointer, 0L, SEEK_SET);
    tree->status = '0';
    fwrite("0", sizeof(char), 1, tree->arvorePointer);

    // Atualiza a struct binary para modificar o arquivo
    fseek(linhaPointer, 0L, SEEK_SET);
    fwrite("0", sizeof(char), 1, linhaPointer);
    fread(&b->byteProxReg, sizeof(long long int), 1, linhaPointer);
    fread(&b->nroRegistros, sizeof(int), 1, linhaPointer);
    fread(&b->nroRegRemovidos, sizeof(int), 1, linhaPointer);
    fseek(linhaPointer, 0L, SEEK_END);
    
    Linha *line = (Linha *)malloc(sizeof(Linha));    // Struct usada para armazenar os dados de um veículo por vez
    line->nomeLinha = (char*)malloc(sizeof(char)*64);
    line->corLinha = (char*)malloc(sizeof(char)*64);

    for(int i = 0; i < n; i++){
        //leitura dos dados a serem inseridos
        scanf("%d", &line->codLinha);
        scan_quote_string(&line->aceitaCartao);
        scan_quote_string(line->nomeLinha);
        scan_quote_string(line->corLinha);

        
        line->tamanhoNome = strlen(line->nomeLinha);
        line->tamanhoCor = strlen(line->corLinha);
        
        line->tamanhoRegistro = STATIC_LINE_SIZE + line->tamanhoNome + line->tamanhoCor;

        line->removido = '1';
        
        int C = line->codLinha;
        long Pr = ftell(linhaPointer);

        // Escreve o registro nos arquivos
        escreveBinLinhas(linhaPointer, line);
        insertion(&tree, C, Pr);

        b->nroRegistros++;
        b->byteProxReg = ftell(linhaPointer);
    }

    // Atualiza o cabeçalho
    b->status = '1';
    b->byteProxReg = ftell(linhaPointer);
    escreveCabecalhoLinhas(linhaPointer, b);

    free(line->nomeLinha);
    free(line->corLinha);

    free(line);
    free(b);
    
    // Atualiza o status
    tree->status = '1';
    escreveCabecalhoArvoreB(tree);

    ftree(tree);
    fclose(linhaPointer);
    
    binarioNaTela(arvore);
}

void funcionalidade16(){
    char letter1, letter2, letter3;
    Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));    // Struct usada para armazenar os dados de um veículo por vez

    char veiculoArquivoNome[64];
    scanf("%s", veiculoArquivoNome);

    FILE *veiculoPointer = fopen(veiculoArquivoNome, "rb");     // Abre em modo de apenas leitura de arquivo binário

    // Caso o arquivo solicitado não exista, imprime erro e retorna
    if(veiculoPointer == NULL){
        printf("Falha no processamento do arquivo.\n");
        free(v);

        return;
    }

    if(fread(&letter1, sizeof(char), 1, veiculoPointer) == 0){
        printf("Registro inexistente.\n");
        free(v);
        fclose(veiculoPointer);

        return;
    }else fseek(veiculoPointer, -1L, SEEK_CUR); // Caso contrário, retorna um byte no cursor

    FILE *linhaPointer = NULL, *indicePointer = NULL;
    char linha[32], indice[32];

    scanf("%s", linha);
    char temp[9];
    scanf("%s", temp);
    scanf("%s", temp);

    scanf("%s", indice);
    linhaPointer = fopen(linha, "rb");
    indicePointer = fopen(indice, "rb");

    //testes de falha de processamento
    if((linhaPointer == NULL) | (indicePointer == NULL)){
        printf("Falha no processamento do arquivo.\n");
        free(v);
        fclose(linhaPointer);

        return;
    }
    fread(&letter1, sizeof(char), 1, veiculoPointer);
    fread(&letter2, sizeof(char), 1, indicePointer);
    fread(&letter3, sizeof(char), 1, linhaPointer);
    if((letter1 == '0') | (letter2 == '0') | (letter3 == '0')){
        printf("Falha no processamento do arquivo.\n");
        free(v);
        fclose(linhaPointer);
        fclose(veiculoPointer);
        fclose(indicePointer);
        return;
    }

    int noRaiz;
    fread(&noRaiz, sizeof(int), 1, indicePointer);

    fseek(indicePointer, noRaiz, SEEK_SET);
    Page *pagina = (Page*)malloc(sizeof(Page));

    fseek(veiculoPointer, 175L, SEEK_SET);
    Linha *line;
    
    v->categoria = (char *)malloc(256*sizeof(char));
    v->modelo = (char *)malloc(256*sizeof(char));
    
    char houveMatch = '0';
    while(fread(&letter1, sizeof(char), 1, veiculoPointer) != 0){    // Armazena em letter a primeira letra da linha para conferir se o cursor está no final
        v->removido = letter1;   // O primeiro byte já é o "removido"

        fread(&v->tamanhoRegistro, sizeof(int), 1, veiculoPointer);

        // Caso removido seja '0', já é pulado o registro
        if(v->removido == '0') fseek(veiculoPointer, v->tamanhoRegistro, SEEK_CUR);
        else{
            fread(v->prefixo, sizeof(char), 5, veiculoPointer);
            v->prefixo[5] = '\0';  // Insere '\0' para a impressão


            fread(v->data, sizeof(char), 10, veiculoPointer);
            v->data[10] = 0;
            
            fread(&v->quantidadeLugares, sizeof(int), 1, veiculoPointer);
            
            fread(&v->codLinha, sizeof(int), 1, veiculoPointer);
            
            // Caso o tamanho do modelo seja 0, logo, o modelo é nulo
            fread(&v->tamanhoModelo, sizeof(int), 1, veiculoPointer);
            if(v->tamanhoModelo > 0){
                fread(v->modelo, sizeof(char), v->tamanhoModelo, veiculoPointer);
            }
            v->modelo[v->tamanhoModelo] = '\0';

            // Segue o exemplo do registro acima                
            fread(&v->tamanhoCategoria, sizeof(int), 1, veiculoPointer);
            if(v->tamanhoCategoria > 0){
                fread(v->categoria, sizeof(char), v->tamanhoCategoria, veiculoPointer);
            }
            v->categoria[v->tamanhoCategoria] = '\0';

            fseek(indicePointer, 1L, SEEK_SET);
            fread(&noRaiz, sizeof(int), 1, indicePointer);

            long long RRN = 0;
            RRN = buscaBinaria(pagina, indicePointer, v->codLinha, noRaiz);

            if(RRN != -1){
		//se for encontrado match, o programa salva que foi encontrado, le a linha correspondente e imprime os dados
                houveMatch = 1;
                line = leLinha(linhaPointer, RRN);

                printVeiculoMatch(v);
                printLinhaMatch(line);

                free(line->nomeLinha);
                free(line->corLinha);
                free(line);
            }
        }
    }

    if(houveMatch == '0') printf("Registro inexistente.\n"); //caso especial em que não há match, por isso a variável houveMatch
    free(v->modelo);
    free(v->categoria);
    free(v);
    free(pagina);
    fclose(veiculoPointer);
    fclose(linhaPointer);
    fclose(indicePointer);
}

int funcionalidade18(char *linhaDesordenadaNome, char *linhaOrdenadaNome, char *campo){
    char letter;
	//inicializando a variável de teste e testando a consistencia dos arquivos

    FILE *original = fopen(linhaDesordenadaNome, "rb");

    if((original == NULL)){
        printf("Falha no processamento do arquivo.\n");
        return 0;
    }
    
    fread(&letter, sizeof(char), 1, original);
    if(letter == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(original);
        return 0;
    }
    
    FILE *ordenado = fopen(linhaOrdenadaNome, "wb+");

    fseek(original, 9L, SEEK_SET);
    int numberOfRegisters = 0;
    fread(&numberOfRegisters, sizeof(int), 1, original);
    
    int i = 0;
    int removidos = 0;
	//o ponteiro duplo **line é uma matriz de linhas, será usada para ordená-las de acordo com codLinha
    Linha **line = (Linha**)malloc(numberOfRegisters * sizeof(Linha*));

    fseek(original, 82L, SEEK_SET);
    while(fread(&letter, sizeof(char), 1, original) != 0){
        line[i] = (Linha*)malloc(sizeof(Linha));
        fread(&line[i]->tamanhoRegistro, sizeof(int), 1, original);

        if(letter == '0'){
            fseek(original, line[i]->tamanhoRegistro, SEEK_CUR);
		//se certa linha for removida, o contador volta um número para sobrescrever a posição
		free(line[i]);
            i--;
            removidos++;
        }else{
            line[i]->removido = '1';
            //caso a linha não seja removida, será lida e printada ao fim desse condicional else
            fread(&line[i]->codLinha, sizeof(int), 1, original);
            fread(&line[i]->aceitaCartao, sizeof(char), 1, original);
            fread(&line[i]->tamanhoNome, sizeof(int), 1, original);


            if(line[i]->tamanhoNome != 0){
                line[i]->nomeLinha = (char*)malloc(sizeof(char)*line[i]->tamanhoNome);
                fread(line[i]->nomeLinha, line[i]->tamanhoNome, 1, original);
            }
            else line[i]->nomeLinha = NULL;

            fread(&line[i]->tamanhoCor, sizeof(int), 1, original);
            if(line[i]->tamanhoCor != 0){
                line[i]->corLinha = (char*)malloc(sizeof(char) * line[i]->tamanhoCor);
                fread(line[i]->corLinha, line[i]->tamanhoCor, 1, original);
            }
        }
        i++;
    }
	//copiando o cabecalho que será editado depois
    char cabecalho[82];
    fseek(original, 0L, SEEK_SET);
    fread(cabecalho, 82, sizeof(char), original);
    fwrite(cabecalho, 82, sizeof(char), ordenado);

    qsort(line, numberOfRegisters, sizeof(Linha*), compareLinhas);

	//escrevendo as linhas já ordenadas
    for(int b = 0; b < numberOfRegisters; b++){
        escreveBinLinhas(ordenado, line[b]);
    }

	//liberando o espaço alocado
    for(int i = 0; i < numberOfRegisters; i++){
        free(line[i]->nomeLinha);
        free(line[i]->corLinha);
        free(line[i]);
    }
    free(line);

	//atualizando o número de removidos e o byteProxReg
    Binary *bin = leCabecalho(original);
    bin->nroRegRemovidos = 0;
    bin->byteProxReg = ftell(ordenado);
    escreveCabecalhoLinhas(ordenado, bin);

    free(bin);
    fclose(original);
    fclose(ordenado);

    return 1;
}

int compareLinhas(const void *a, const void *b){
	//funcao usada pelo sort para comparar as linhas
    Linha *linhaA = *(Linha**)a;
    Linha *linhaB = *(Linha**)b;

    return (linhaA->codLinha - linhaB->codLinha);
}

void leLinhaEImprime(FILE *fp, long long Pr){
    if(Pr == -1){
        printf("Registro inexistente.\n");
        return;
    }
    Linha *line = (Linha*)malloc(sizeof(Linha));

    fseek(fp, Pr, SEEK_SET);

    fread(&line->removido, sizeof(char), 1, fp);

    if(line->removido == '0'){
        free(line);
        return;
    }

    line->nomeLinha = (char*)malloc(sizeof(char)*256);
    line->corLinha = (char*)malloc(sizeof(char)*256);

    fread(&line->tamanhoRegistro, sizeof(int), 1, fp);
    fread(&line->codLinha, sizeof(int), 1, fp);
    fread(&line->aceitaCartao, sizeof(char), 1, fp);
    fread(&line->tamanhoNome, sizeof(int), 1, fp);

    if(line->tamanhoNome != 0)fread(line->nomeLinha, line->tamanhoNome, 1, fp);
    else line->nomeLinha[0] = '\0';

    fread(&line->tamanhoCor, sizeof(int), 1, fp);
    fread(line->corLinha, line->tamanhoCor, 1, fp);
    
    line->corLinha[line->tamanhoCor] = '\0';

    printBusLine(fp, line);

    free(line->nomeLinha);
    free(line->corLinha);
    free(line);
    return;
}

Linha *leLinha(FILE *fp, long long Pr){
    if(Pr == -1){
        return NULL;
    }
    Linha *line = (Linha*)malloc(sizeof(Linha));

    fseek(fp, Pr, SEEK_SET);

    fread(&line->removido, sizeof(char), 1, fp);

    if(line->removido == '0'){
        free(line);
        return NULL;
    }

    line->corLinha = (char*)malloc(sizeof(char)*256);
    line->nomeLinha = (char*)malloc(sizeof(char)*256);

    fread(&line->tamanhoRegistro, sizeof(int), 1, fp);
    fread(&line->codLinha, sizeof(int), 1, fp);
    fread(&line->aceitaCartao, sizeof(char), 1, fp);
    fread(&line->tamanhoNome, sizeof(int), 1, fp);

    if(line->tamanhoNome != 0){
        fread(line->nomeLinha, sizeof(char), line->tamanhoNome, fp);
        line->nomeLinha[line->tamanhoNome] = '\0';
    }else line->nomeLinha[0] = '\0';

    fread(&line->tamanhoCor, sizeof(int), 1, fp);
    fread(line->corLinha, line->tamanhoCor, 1, fp);
    line->corLinha[line->tamanhoCor] = '\0';
    
    line->corLinha[line->tamanhoCor] = '\0';

    return line;
}

void escreveBinLinhas(FILE *fp, Linha *line){
    //escreve as linhas em formato binário
    fwrite(&line->removido, sizeof(char), 1, fp);
    fwrite(&line->tamanhoRegistro, sizeof(int), 1, fp);
    fwrite(&line->codLinha, sizeof(int), 1, fp);
    fwrite(&line->aceitaCartao, sizeof(char), 1, fp);
    fwrite(&line->tamanhoNome, sizeof(int), 1, fp);
    fwrite(line->nomeLinha, sizeof(char), line->tamanhoNome, fp);
    fwrite(&line->tamanhoCor, sizeof(int), 1, fp);
    fwrite(line->corLinha, sizeof(char), line->tamanhoCor, fp);
}

char *cartao(char type){
    //atribui as descrições para os dados de cartão
    switch(type){
        case 'S':
        return "PAGAMENTO SOMENTE COM CARTAO SEM PRESENCA DE COBRADOR";
        break;
        case 'F':
        return "PAGAMENTO EM CARTAO SOMENTE NO FINAL DE SEMANA";
        break;
        case 'N':
        return "PAGAMENTO EM CARTAO E DINHEIRO";
    }

    return NULL;
}

void printBusLine(FILE *fp, Linha *line){
    //imprime as informações da linha

    //Garantia de que os nomes da linha e da cor não terão overflow sobre os dados
    if(line->tamanhoNome != 0)line->nomeLinha[line->tamanhoNome] = '\0';
    if(line->tamanhoCor != 0) line->corLinha[line->tamanhoCor] = '\0';

    printf("Codigo da linha: %d\n", line->codLinha);
    printf("Nome da linha: %s\n", line->tamanhoNome != 0 ? line->nomeLinha : "campo com valor nulo");
    printf("Cor que descreve a linha: %s\n", line->tamanhoCor != 0 ? line->corLinha: "campo com valor nulo");
    printf("Aceita cartao: %s\n\n", cartao(line->aceitaCartao));
}

int findRegister(Linha *line, char *nomeDoCampo, char *valor){
    //função que busca os campos procurados. Os ifs externos encontram o campo procurado e os internos buscam matches do campo escolhido,
    //retornando 1 caso haja um match
    if(strcmp(nomeDoCampo, "codLinha") == 0){
        if(atoi(valor) == line->codLinha){
            return 1;
        }
    }
    else if(strcmp(nomeDoCampo, "aceitaCartao") == 0){
        if(strcmp(valor, &line->aceitaCartao) == 0){
            return 1;
        }
    }
    else if(strcmp(nomeDoCampo, "nomeLinha") == 0){
        if(strcmp(valor, line->nomeLinha) == 0){
            return 1;
        }
    }
    else if(strcmp(nomeDoCampo, "corLinha") == 0){
        if(strcmp(valor, line->corLinha) == 0){
            return 1;
        }
    }

    return 0;
}

long long buscaBinaria(Page *pagina, FILE *fp, int chave, long long RRN){
    if(RRN == -1){
        return -1;
    }

    fseek(fp, (RRN+1)*77, SEEK_SET);

    fread(&pagina->folha, sizeof(char), 1, fp);
    fread(&pagina->nroChavesIndexadas, sizeof(int), 1, fp);
    fread(&pagina->RRNdoNo, sizeof(int), 1, fp);

    for(int i = 0; i < pagina->nroChavesIndexadas; i++){
        fread(&pagina->P[i], sizeof(int), 1, fp);
        fread(&pagina->C[i], sizeof(int), 1, fp);
        fread(&pagina->Pr[i], sizeof(long long), 1, fp);
    }

    fread(&pagina->P[pagina->nroChavesIndexadas], sizeof(int), 1, fp);

    int i = 0;
    while(i < pagina->nroChavesIndexadas && chave > pagina->C[i]){
        i++;
    }

    if(pagina->C[i] == chave){
        return pagina->Pr[i];
    }

    if(pagina->folha == '1'){
        return -1;
    }

    return buscaBinaria(pagina, fp, chave, pagina->P[i]);
}

void printLinhaMatch(Linha *line){
    printf("Codigo da linha: %d\n", line->codLinha);
    printf("Nome da linha: %s\n", line->tamanhoNome != 0 ? line->nomeLinha : "campo com valor nulo");
    printf("Cor que descreve a linha: %s\n", line->corLinha);
    printf("Aceita cartao: %s\n\n", cartao(line->aceitaCartao));
}
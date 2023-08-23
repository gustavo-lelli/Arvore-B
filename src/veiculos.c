#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"veiculos.h"
#include"linhas.h"
#include"file.h"
#include"arvore-b.h"

void funcionalidade1(){
    char letter, csv[32], bin[32], quantidadeLugaresTemp[5], codLinhaTemp[5];    
    FILE *csvPointer, *binPointer;

    scanf("%s", csv);
    csvPointer = fopen(csv, "r");       // Abre o arquivo em modo de apenas leitura

    scanf("%s", bin);    
    binPointer = fopen(bin, "wb+");     // Abre em modo de criação e escrita de arquivo binário

    if(csvPointer == NULL){ //caso em que o arquivo csv não existe
        printf("Falha no processamento do arquivo.\n");
        fclose(binPointer);
        return;
    }
    Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));    // Struct usada para armazenar os dados de um veículo por vez
    Binary *b = (Binary *)malloc(sizeof(Binary));       // Struct usada para salvar as informações do arq. binario
    
    //o espaço para o modelo e categoria são alocados uma vez só, e liberados no fim
    //esses char* serão reutilizados a cada leitura e escrita
    v->modelo = (char*)malloc(sizeof(char)*64);
    v->categoria = (char*)malloc(sizeof(char)*64);

    // Já coloca os valores iniciais na struct do binário, com o valor de inconsistência
    b->status = '0';
    b->byteProxReg = 175;
    b->nroRegistros = 0;
    b->nroRegRemovidos = 0;

    escreveCabecalhoVeiculos(binPointer, b); // Função que escreveo cabeçalho no arquivo binário 

    fseek(csvPointer, 164L, SEEK_SET);  // Pula o cabeçalho do .csv
    while(fread(&letter, sizeof(char), 1, csvPointer) != 0){    // Armazena em letter a primeira letra da linha para conferir se o cursor está no final
        if(letter == '*'){      // Aproveitando que letter armazena a primera letra, dá para saber se o arquivo deve ser removido
            v->removido = '0';
            b->nroRegRemovidos++;
        }else{
            v->removido = '1';
            fseek(csvPointer, -1L, SEEK_CUR);
            b->nroRegistros++;
        }
        
        fscanf(csvPointer, "%[^,]", v->prefixo);
        fgetc(csvPointer);  // Descarta a vírgula entre as categorias

        fscanf(csvPointer, "%[^,]", v->data);
        fgetc(csvPointer);

        // Caso data seja nula, preenche com o lixo
        if(!strncmp(v->data, "NULO", 4)){
            strcpy(v->data, "@@@@@@@@@@");
            v->data[0] = '\0';
        }

        // Usa uma string temporária para conferir se é nula ou não
        fscanf(csvPointer, "%[^,]", quantidadeLugaresTemp);
        fgetc(csvPointer);
        
        // Caso seja nulo, atribui -1, caso contrário, preenche com o valor propriamente escaneado
        if(!strncmp(quantidadeLugaresTemp, "NULO", 4)) v->quantidadeLugares = -1;
        else v->quantidadeLugares = atoi(quantidadeLugaresTemp);
        
        // Segue a mesma linha de raciocínio do acima
        fscanf(csvPointer, "%[^,]", codLinhaTemp);
        fgetc(csvPointer);

        if(!strncmp(codLinhaTemp, "NULO", 4)) v->codLinha = -1;
        else v->codLinha = atoi(codLinhaTemp);

        // Utiliza uma função que escreve e conta os caracteres ao mesmo tempo        
        freadline(csvPointer, v->modelo);
        v->tamanhoModelo = strlen(v->modelo);
        
        freadline(csvPointer, v->categoria);
        v->tamanhoCategoria = strlen(v->categoria);

        v->tamanhoRegistro = STATIC_VEHICLE_SIZE + v->tamanhoModelo + v->tamanhoCategoria;

        // Escreve as informações adquiridas no .bin
        escreveBinVeiculos(binPointer, v);

        b->byteProxReg = ftell(binPointer); // Atualiza o indicador do próximo registro para o final que está sendo escrito
    }
    b->status = '1';
    escreveCabecalhoVeiculos(binPointer, b);   // Atualiza o cabeçalho no final do programa
    free(v->categoria);
    free(v->modelo);
    free(v);
    free(b);
    fclose(binPointer);
    fclose(csvPointer);

    binarioNaTela(bin);

}

/*
Para não precisar usar fseek sempre que for imprimir uma categoria, serão salvas as categorias do cabeçalho nas variáveis:
cab1: prefixo
cab2: data
cab3: quantidade de lugares
cab4: modelo
cab5: categoria
*/

void funcionalidade3(){
    char letter, bin[32], *new_data;
    char *cab1 = (char *)malloc(19 * sizeof(char)), *cab2 = (char *)malloc(36 * sizeof(char)), *cab3 = (char *)malloc(43 * sizeof(char)), *cab4 = (char *)malloc(18 * sizeof(char)), *cab5 = (char *)malloc(21 * sizeof(char));
    FILE *binPointer;
    Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));    // Struct usada para armazenar os dados de um veículo por vez

    scanf("%s", bin);  
    binPointer = fopen(bin, "rb");     // Abre em modo de apenas leitura de arquivo binário

    // Caso o arquivo solicitado não exista, imprime erro e retorna
    fread(&letter, sizeof(char), 1, binPointer);
    if(binPointer == NULL || letter == '0'){
        printf("Falha no processamento do arquivo.\n");

        free(cab1);
        free(cab2);
        free(cab3);
        free(cab4);
        free(cab5);
        free(v);

        return;
    }

    // Lê e salva o cabecalho
    fseek(binPointer, 17, SEEK_SET);
    salvaCabecalho(binPointer, &cab1, &cab2, &cab3, &cab4, &cab5);

    // Caso o primeiro byte após o cabecalho seja nulo, significa que ele não possui registros
    // Imprime erro e retorna
    if(fread(&letter, sizeof(char), 1, binPointer) == 0){
        printf("Registro inexistente.\n");
        
        free(cab1);
        free(cab2);
        free(cab3);
        free(cab4);
        free(cab5);
        free(v);
        fclose(binPointer);

        return;
    }else fseek(binPointer, -1L, SEEK_CUR); // Caso contrário, retorna um byte no cursor

    while(fread(&letter, sizeof(char), 1, binPointer) != 0){    // Armazena em letter a primeira letra da linha para conferir se o cursor está no final
        v->removido = letter;   // O primeiro byte já é o "removido"

        fread(&v->tamanhoRegistro, sizeof(int), 1, binPointer);

        // Caso removido seja '0', já é pulado o registro
        if(v->removido == '0') fseek(binPointer, v->tamanhoRegistro, SEEK_CUR);
        else{
            fread(v->prefixo, sizeof(char), 5, binPointer);
            v->prefixo[5] = 0;  // Insere '\0' para a impressão
            printf("%s: %s\n", cab1, v->prefixo);

            fread(v->data, sizeof(char), 10, binPointer);
            v->data[10] = 0;
            
            fread(&v->quantidadeLugares, sizeof(int), 1, binPointer);
            
            fseek(binPointer, 4, SEEK_CUR); // Pula o codLinha
            
            // Caso o tamanho do modelo seja 0, logo, o modelo é nulo
            fread(&v->tamanhoModelo, sizeof(int), 1, binPointer);
            if(v->tamanhoModelo <= 0) printf("%s: campo com valor nulo\n", cab4);
            else{
                v->modelo = (char *)malloc((v->tamanhoModelo + 1) * sizeof(char));
                fread(v->modelo, sizeof(char), v->tamanhoModelo, binPointer);
                v->modelo[v->tamanhoModelo] = 0;
                printf("%s: %s\n", cab4, v->modelo);
            }

            // Segue o exemplo do registro acima                
            fread(&v->tamanhoCategoria, sizeof(int), 1, binPointer);
            if(v->tamanhoCategoria <= 0) printf("%s: campo com valor nulo\n", cab5);
            else{
                v->categoria = (char *)malloc((v->tamanhoCategoria + 1) * sizeof(char));
                fread(v->categoria, sizeof(char), v->tamanhoCategoria, binPointer);
                v->categoria[v->tamanhoCategoria] = 0;
                printf("%s: %s\n", cab5, v->categoria);
            }

            // Caso data possua '@' (lixo), significa que ele é nulo
            if(v->data[1] == '@') printf("%s: campo com valor nulo\n", cab2);
            else{
                // Caso contrário, a data é convertida
                new_data = converteData(v->data);
                printf("%s: %s\n", cab2, new_data);
                free(new_data);
            }

            if(v->quantidadeLugares == -1) printf("%s: campo com valor nulo\n", cab3);
            else printf("%s: %d\n", cab3, v->quantidadeLugares);

            printf("\n");

            if(v->tamanhoModelo > 0) free(v->modelo);
            if(v->tamanhoCategoria > 0) free(v->categoria);
        }
    }

    free(cab1);
    free(cab2);
    free(cab3);
    free(cab4);
    free(cab5);
    free(v);
    fclose(binPointer);
}

void funcionalidade5(){
    int encontrado = 0; // Variável com o objetivo de ser boolean, que serve para mostrar a mensagem de erro
    char letter, bin[32], *new_data, nomeDoCampo[32], valor[64];    
    FILE *binPointer;
    Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));    // Struct usada para armazenar os dados de um veículo por vez

    scanf("%s", bin);  
    binPointer = fopen(bin, "rb");     // Abre em modo de leitura de arquivo binário

    scanf("%s", nomeDoCampo);

    scan_quote_string(valor);   // Usa-se a função fornecida para retirar as aspas

    // Caso o arquivo solicitado não exista, imprime erro e retorna
    fread(&letter, sizeof(char), 1, binPointer);
    if(binPointer == NULL || letter == '0'){
        printf("Falha no processamento do arquivo.\n");
        free(v);

        return;
    }

    char *cab1 = (char *)malloc(19 * sizeof(char)), *cab2 = (char *)malloc(36 * sizeof(char)), *cab3 = (char *)malloc(43 * sizeof(char)), *cab4 = (char *)malloc(18 * sizeof(char)), *cab5 = (char *)malloc(21 * sizeof(char));
    // Lê e salva o cabecalho
    fseek(binPointer, 17, SEEK_SET);
    salvaCabecalho(binPointer, &cab1, &cab2, &cab3, &cab4, &cab5);

    // Caso o primeiro byte após o cabecalho seja nulo, significa que ele não possui registros
    // Imprime erro e retorna
    if(fread(&letter, sizeof(char), 1, binPointer) == 0){
        printf("Registro inexistente.\n");
        
        free(cab1);
        free(cab2);
        free(cab3);
        free(cab4);
        free(cab5);
        free(v);
        fclose(binPointer);

        return;
    }else fseek(binPointer, -1L, SEEK_CUR); // Caso contrário, retorna um byte no cursor

    while(fread(&letter, sizeof(char), 1, binPointer) != 0){    // Armazena em letter a primeira letra da linha para conferir se o cursor está no final
        v->removido = letter;   // O primeiro byte já é o "removido"

        fread(&v->tamanhoRegistro, sizeof(int), 1, binPointer);

        // Caso removido seja '0', já é pulado o registro
        if(v->removido == '0') fseek(binPointer, v->tamanhoRegistro, SEEK_CUR);
        else{
            fread(v->prefixo, sizeof(char), 5, binPointer);

            // Caso data possua '@' (lixo), significa que ele é nulo
            fread(v->data, sizeof(char), 10, binPointer);
            v->data[10] = 0;
            if(v->data[1] != '@'){
                // Caso contrário, a data é convertida
                new_data = converteData(v->data);
            }
            
            fread(&v->quantidadeLugares, sizeof(int), 1, binPointer);
            
            fseek(binPointer, 4, SEEK_CUR); // Pula o codLinha
            
            // Caso o tamanho do modelo seja 0, logo, o modelo é nulo
            fread(&v->tamanhoModelo, sizeof(int), 1, binPointer);
            if(v->tamanhoModelo > 0){
                v->modelo = (char *)malloc((v->tamanhoModelo + 1) * sizeof(char));
                fread(v->modelo, sizeof(char), v->tamanhoModelo, binPointer);
                v->modelo[v->tamanhoModelo] = 0;
            }

            // Segue o exemplo do registro acima                
            fread(&v->tamanhoCategoria, sizeof(int), 1, binPointer);
            if(v->tamanhoCategoria > 0){
                v->categoria = (char *)malloc((v->tamanhoCategoria + 1) * sizeof(char));
                fread(v->categoria, sizeof(char), v->tamanhoCategoria, binPointer);
                v->categoria[v->tamanhoCategoria] = 0;
            }
            
            v->prefixo[5] = 0;  // Insere '\0' para a impressão

            // Caso alguma categoria corresponda ao "valor" pedido, imprimirá as informações
            // A data, a quantidadeLugares, o modelo e acategoria não podem ser nulos na comparação
            if(!strncmp(valor, v->prefixo, 5) || (v->data[1] != '@' && !strcmp(valor, new_data)) || atoi(valor) == v->quantidadeLugares || (v->tamanhoModelo > 0 && !strcmp(valor, v->modelo)) || (v->tamanhoCategoria > 0 && !strcmp(valor, v->categoria))){
                printf("%s: %s\n", cab1, v->prefixo);
            
                if(v->tamanhoModelo <= 0) printf("%s: campo com valor nulo\n", cab4);
                else printf("%s: %s\n", cab4, v->modelo);
            
                if(v->tamanhoCategoria <= 0) printf("%s: campo com valor nulo\n", cab5);
                else printf("%s: %s\n", cab5, v->categoria);
            
                // Caso data possua '@' (lixo), significa que ele é nulo
                if(v->data[1] == '@') printf("%s: campo com valor nulo\n", cab2);
                else printf("%s: %s\n", cab2, new_data);

                if(v->quantidadeLugares == -1) printf("%s: campo com valor nulo\n", cab3);
                else printf("%s: %d\n", cab3, v->quantidadeLugares);
                
                printf("\n");

                encontrado = 1; // Depois de encontrado e printado, não poderá mais ocorrer a mensagem de registro não encontrado
            }

            if(v->tamanhoModelo > 0) free(v->modelo);
            if(v->tamanhoCategoria > 0) free(v->categoria);
            if(v->data[1] != '@') free(new_data);
        }
    }

    // Caso não tenha sido encontrado ou o registro tenha sido logicamente removido, printa o erro
    if(encontrado == 0) printf("Registro inexistente.\n");

    free(cab1);
    free(cab2);
    free(cab3);
    free(cab4);
    free(cab5);
    free(v);
    fclose(binPointer);
}

void funcionalidade7(){
    int n;
    char letter, bin[32], quantidadeLugaresTemp[5], codLinhaTemp[5];
    FILE *binPointer;
    Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));    // Struct usada para armazenar os dados de um veículo por vez
    Binary *b = (Binary *)malloc(sizeof(Binary));       // Struct usada para salvar as informações do arq. binario

    scanf("%s", bin);
    binPointer = fopen(bin, "rb+");  // Abre o arquivo em modo de leitura e escrita binaria
    
    scanf("%d", &n);

    // Verifica se o arquivo existe e se está com o status consistente
    fread(&letter, sizeof(char), 1, binPointer);

    
    if(binPointer == NULL){
        printf("Falha no processamento do arquivo.\n");
        free(v);
        free(b);

        return;
    }
    else if(letter == '0'){
        printf("Falha no processamento do arquivo.\n");
        free(v);
        free(b);
        fclose(binPointer);
        return;
    }
    
    // Atualiza a struct binary para modificar o arquivo
    fseek(binPointer, 0L, SEEK_SET);
    fwrite("0", sizeof(char), 1, binPointer);
    fread(&b->byteProxReg, sizeof(long long int), 1, binPointer);
    fread(&b->nroRegistros, sizeof(int), 1, binPointer);
    fread(&b->nroRegRemovidos, sizeof(int), 1, binPointer);
    fseek(binPointer, 0L, SEEK_END);

    v->categoria = (char*)malloc(sizeof(char)*64);
    v->modelo = (char*)malloc(sizeof(char)*64);
    for(int i = 0; i < n; i++){
        scanf("%*c"); // Essa função está sendo usada para descartar 
        scanf("%*c");
        scanf("%[^\"]", v->prefixo);
        scanf("%*c");
        scanf("%*c");

        letter = getchar();
        if(letter == '"'){  // Caso possua '"', não é nulo
            scanf("%[^\"]", v->data);
        }else{  // Caso nulo, coloca-se lixo
            scanf("%s", v->data);
            strcpy(v->data, "@@@@@@@@@@");
            v->data[0] = '\0';
        }
        scanf("%*c");

        scanf("%s ", quantidadeLugaresTemp);
        if(!strncmp(quantidadeLugaresTemp, "NULO", 4)) v->quantidadeLugares = -1;
        else v->quantidadeLugares = atoi(quantidadeLugaresTemp);
        
        scanf("%s ", codLinhaTemp);
        if(!strncmp(codLinhaTemp, "NULO", 4)) v->codLinha = -1;
        else v->codLinha = atoi(codLinhaTemp);
        
        scan_quote_string(v->modelo);
        v->tamanhoModelo = strlen(v->modelo);

        scan_quote_string(v->categoria);
        v->tamanhoCategoria = strlen(v->categoria);

        v->tamanhoRegistro = STATIC_VEHICLE_SIZE + v->tamanhoModelo + v->tamanhoCategoria;

        v->removido = '1';
        // Escreve o registro no arquivo
        escreveBinVeiculos(binPointer, v);

        b->nroRegistros++;
        b->byteProxReg = ftell(binPointer);
    }

    // Atualiza o cabeçalho
    b->status = '1';
    escreveCabecalhoVeiculos(binPointer, b);

    free(v->categoria);
    free(v->modelo);

    free(v);
    free(b);
    fclose(binPointer);

    binarioNaTela(bin);
}

void funcionalidade9(){
    int C, jump;
    long long Pr;
    char removido, veiculo[32], arvore[32], prefixo[6];    
    FILE *veiculoPointer;
    Btree *tree;

    scanf("%s", veiculo);
    veiculoPointer = fopen(veiculo, "rb");  // Abre o arquivo em modo de apenas leitura

    scanf("%s", arvore);    

    if(veiculoPointer == NULL){ // Caso o arquivo veiculo.bin não exista
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Caso o arquivo esteja inconsistente, retorna com erro
    fread(&removido, sizeof(char), 1, veiculoPointer);
    if(removido == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(veiculoPointer);
        return;
    }

    fseek(veiculoPointer, 175L, SEEK_SET);  // Pula o cabeçalho do veiculo.bin
    while(fread(&removido, sizeof(char), 1, veiculoPointer) == '0'){    // Armazena o valor do campo 'removido' para comparar e escolher um nó não nulo para colocar na raiz
        fread(&jump, sizeof(int), 1, veiculoPointer);   // Salva o tamanho do registro, para conseguir pulá-lo
        fseek(veiculoPointer, jump, SEEK_CUR);
    }
    
    fseek(veiculoPointer, -1L, SEEK_CUR);   // Retorna um byte para pegar o Pr correto
    Pr = ftell(veiculoPointer);
            
    fseek(veiculoPointer, 1L, SEEK_CUR);
    fread(&jump, sizeof(int), 1, veiculoPointer);   // Salva o tamanho do registro para pulá-lo

    fread(prefixo, sizeof(char), 5, veiculoPointer);

    C = convertePrefixo(prefixo);   // Converte o prefixo para um núero inteiro

    tree = createBTree(arvore, C, Pr);    // Struct usada para salvar as informações da Árvore-B
    escreveCabecalhoArvoreB(tree);  // Função que escreve o cabeçalho no arquivo da Árvore-B
    escreveBinArvoreB(tree->root, tree->arvorePointer, 0);    // Função que escreve uma página no arquivo da Árvore-B
    froot(tree);
    
    fseek(veiculoPointer, (jump - 5), SEEK_CUR);
    while(fread(&removido, sizeof(char), 1, veiculoPointer) != 0){    // Armazena o primeiro byte da linha, para que se saiba se está no fim do arquivo
        if(removido == '1'){      // Aproveitando que 'removido' armazena a primera letra, dá para saber se a página está logicamente removida
            fseek(veiculoPointer, -1L, SEEK_CUR);   // Retona um byte para pegar o Pr correto
            Pr = ftell(veiculoPointer);
            
            fseek(veiculoPointer, 1L, SEEK_CUR);
            fread(&jump, sizeof(int), 1, veiculoPointer);    // Salva o tamanho do registro para pulá-lo

            fread(prefixo, sizeof(char), 5, veiculoPointer);
            prefixo[5] = '\0';

            C = convertePrefixo(prefixo);   // Converte o prefixo para um número inteiro

            insertion(&tree, C, Pr); 
    
            fseek(veiculoPointer, (jump - 5), SEEK_CUR);
        }else{
            fread(&jump, sizeof(int), 1, veiculoPointer);
            fseek(veiculoPointer, jump, SEEK_CUR);
        }
    }
    tree->status = '1';
    escreveCabecalhoArvoreB(tree);   // Atualiza o cabeçalho no final do programa
    ftree(tree);
    fclose(veiculoPointer);

    binarioNaTela(arvore);
}

void funcionalidade11(){
    char removido, veiculo[32], arvore[32], nomeDoCampo[8], prefixo[6];    
    FILE *veiculoPointer;

    scanf("%s", veiculo);  
    veiculoPointer = fopen(veiculo, "rb");     // Abre em modo de leitura de arquivo binário

    // Verifica se o arquivo existe
    if(veiculoPointer == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    scanf("%s", arvore);
    Btree *tree = loadBTree(arvore, SEARCH);

    // Verifica se o arquivo existe
    if(tree == NULL){
        printf("Falha no processamento do arquivo.\n");
        fclose(veiculoPointer);
        return;
    }
    
    scanf("%s", nomeDoCampo);

    scan_quote_string(prefixo);   // Usa-se a função fornecida para retirar as aspas

    // Caso algum arquivo solicitado esteja inconsistente, imprime erro e retorna
    fread(&removido, sizeof(char), 1, veiculoPointer);
    if(removido == '0' || tree->status == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(veiculoPointer);
        ftree(tree);
        return;
    }

    char *cab1 = (char *)malloc(19 * sizeof(char)), *cab2 = (char *)malloc(36 * sizeof(char)), *cab3 = (char *)malloc(43 * sizeof(char)), *cab4 = (char *)malloc(18 * sizeof(char)), *cab5 = (char *)malloc(21 * sizeof(char));
    // Lê e salva o cabecalho
    fseek(veiculoPointer, 17, SEEK_SET);
    salvaCabecalho(veiculoPointer, &cab1, &cab2, &cab3, &cab4, &cab5);

    // Caso o primeiro byte após o cabecalho seja nulo, significa que ele não possui registros
    // Imprime erro e retorna
    if(fread(&removido, sizeof(char), 1, veiculoPointer) == 0){
        printf("Registro inexistente.\n");
        
        free(cab1);
        free(cab2);
        free(cab3);
        free(cab4);
        free(cab5);
        ftree(tree);
        fclose(veiculoPointer);

        return;
    }else fseek(veiculoPointer, -1L, SEEK_CUR); // Caso contrário, retorna um byte no cursor

    int pos, C = convertePrefixo(prefixo);
    long long Pr;
    char *new_data;

    Pr = search(tree, C, &pos, tree->noRaiz);

    // Caso não tenha sido encontrado ou o registro tenha sido logicamente removido, printa o erro
    if(Pr == NOT_FOUND) printf("Registro inexistente.\n");
    else{
        Veiculo *veiculo = (Veiculo *)malloc(sizeof(Veiculo));    // Struct usada para armazenar os dados de um veículo por vez
        fseek(veiculoPointer, Pr + 10, SEEK_SET);   // Vai até o ByteOffset do veículo e pula os campos: 'removido', o tamanho do registro e o 'prefixo' (que já foi dado)

        // Caso data possua '@' (lixo), significa que ele é nulo
        fread(veiculo->data, sizeof(char), 10, veiculoPointer);
        veiculo->data[10] = '\0';
        if(veiculo->data[1] != '@'){
            // Caso contrário, a data é convertida
            new_data = converteData(veiculo->data);
        }
            
        fread(&veiculo->quantidadeLugares, sizeof(int), 1, veiculoPointer);
            
        fseek(veiculoPointer, 4, SEEK_CUR); // Pula o codLinha
            
        // Caso o tamanho do modelo seja 0, logo, o modelo é nulo
        fread(&veiculo->tamanhoModelo, sizeof(int), 1, veiculoPointer);
        if(veiculo->tamanhoModelo > 0){
            veiculo->modelo = (char *)malloc((veiculo->tamanhoModelo + 1) * sizeof(char));
            fread(veiculo->modelo, sizeof(char), veiculo->tamanhoModelo, veiculoPointer);
            veiculo->modelo[veiculo->tamanhoModelo] = 0;
        }

        // Segue o exemplo do registro acima                
        fread(&veiculo->tamanhoCategoria, sizeof(int), 1, veiculoPointer);
        if(veiculo->tamanhoCategoria > 0){
            veiculo->categoria = (char *)malloc((veiculo->tamanhoCategoria + 1) * sizeof(char));
            fread(veiculo->categoria, sizeof(char), veiculo->tamanhoCategoria, veiculoPointer);
            veiculo->categoria[veiculo->tamanhoCategoria] = 0;
        }
            
        printf("%s: %s\n", cab1, prefixo);
            
        if(veiculo->tamanhoModelo <= 0) printf("%s: campo com valor nulo\n", cab4);
        else printf("%s: %s\n", cab4, veiculo->modelo);
            
        if(veiculo->tamanhoCategoria <= 0) printf("%s: campo com valor nulo\n", cab5);
        else printf("%s: %s\n", cab5, veiculo->categoria);
        
        // Caso data possua '@' (lixo), significa que ele é nulo
        if(veiculo->data[1] == '@') printf("%s: campo com valor nulo\n", cab2);
        else printf("%s: %s\n", cab2, new_data);

        if(veiculo->quantidadeLugares == -1) printf("%s: campo com valor nulo\n", cab3);
        else printf("%s: %d\n", cab3, veiculo->quantidadeLugares);
        
        printf("\n");

        if(veiculo->tamanhoModelo > 0) free(veiculo->modelo);
        if(veiculo->tamanhoCategoria > 0) free(veiculo->categoria);
        if(veiculo->data[1] != '@') free(new_data);
        free(veiculo);
    }

    free(cab1);
    free(cab2);
    free(cab3);
    free(cab4);
    free(cab5);
    ftree(tree);
    fclose(veiculoPointer);
}

void funcionalidade13(){
    int n;
    char removido, veiculoBin[32], arvore[32], quantidadeLugaresTemp[5], codLinhaTemp[5];
    FILE *veiculoPointer;
    Binary *b = (Binary *)malloc(sizeof(Binary));   // Struct usada para salvar as informações do arq. binario

    scanf("%s ", veiculoBin);
    veiculoPointer = fopen(veiculoBin, "rb+");  // Abre o arquivo em modo de append binaria
    
    // Verifica se o arquivo existe
    if(veiculoPointer == NULL){
        printf("Falha no processamento do arquivo.\n");
        free(b);
        return;
    }

    scanf("%s ", arvore);
    Btree *tree = loadBTree(arvore, ADD);   // Carrega o arquivo da árvore para dentro dela

    // Verifica se o arquivo existe
    if(tree == NULL){
        printf("Falha no processamento do arquivo.\n");
        free(b);
        fclose(veiculoPointer);
        return;
    }
    
    scanf("%d", &n);

    // Verifica se os arquivos estão com o status consistente
    fread(&removido, sizeof(char), 1, veiculoPointer);
    if(removido == '0' || tree->status == '0'){
        printf("Falha no processamento do arquivo.\n");
        free(b);
        fclose(veiculoPointer);
        ftree(tree);
        return;
    }
    
    // Atualiza o status da árvore para modificá-la
    fseek(tree->arvorePointer, 0L, SEEK_SET);
    tree->status = '0';
    fwrite("0", sizeof(char), 1, tree->arvorePointer);

    // Atualiza a struct binary para modificar o arquivo
    fseek(veiculoPointer, 0L, SEEK_SET);
    fwrite("0", sizeof(char), 1, veiculoPointer);
    fread(&b->byteProxReg, sizeof(long long int), 1, veiculoPointer);
    fread(&b->nroRegistros, sizeof(int), 1, veiculoPointer);
    fread(&b->nroRegRemovidos, sizeof(int), 1, veiculoPointer);
    fseek(veiculoPointer, 0L, SEEK_END);
    
    Veiculo *veiculo = (Veiculo *)malloc(sizeof(Veiculo));    // Struct usada para armazenar os dados de um veículo por vez
    veiculo->categoria = (char*)malloc(sizeof(char)*64);
    veiculo->modelo = (char*)malloc(sizeof(char)*64);
    
    for(int i = 0; i < n; i++){
        scan_quote_string(veiculo->prefixo);
        scanf("%*c"); // Essa função está sendo usada para descartar 
        veiculo->prefixo[5] = '\0';

        removido = getchar();
        if(removido == '"'){  // Caso possua '"', não é nulo
            scanf("%[^\"]", veiculo->data);
        }else{  // Caso nulo, coloca-se lixo
            scanf("%s", veiculo->data);
            strcpy(veiculo->data, "@@@@@@@@@@");
            veiculo->data[0] = '\0';
        }
        scanf("%*c");

        scanf("%s ", quantidadeLugaresTemp);
        if(!strncmp(quantidadeLugaresTemp, "NULO", 4)) veiculo->quantidadeLugares = -1;
        else veiculo->quantidadeLugares = atoi(quantidadeLugaresTemp);
        
        scanf("%s ", codLinhaTemp);
        if(!strncmp(codLinhaTemp, "NULO", 4)) veiculo->codLinha = -1;
        else veiculo->codLinha = atoi(codLinhaTemp);
        
        scan_quote_string(veiculo->modelo);
        veiculo->tamanhoModelo = strlen(veiculo->modelo);

        scan_quote_string(veiculo->categoria);
        veiculo->tamanhoCategoria = strlen(veiculo->categoria);

        veiculo->tamanhoRegistro = STATIC_VEHICLE_SIZE + veiculo->tamanhoModelo + veiculo->tamanhoCategoria;

        veiculo->removido = '1';
        
        int C = convertePrefixo(veiculo->prefixo);
        long long Pr = ftell(veiculoPointer);

        // Escreve o registro nos arquivos
        escreveBinVeiculos(veiculoPointer, veiculo);
        insertion(&tree, C, Pr);

        b->nroRegistros++;
        b->byteProxReg = ftell(veiculoPointer);
    }

    free(veiculo->categoria);
    free(veiculo->modelo);

    free(veiculo);

    // Atualiza o status
    tree->status = '1';
    escreveCabecalhoArvoreB(tree);
    
    // Atualiza o cabeçalho
    b->status = '1';
    escreveCabecalhoVeiculos(veiculoPointer, b);
    free(b);
    
    ftree(tree);
    fclose(veiculoPointer);

    binarioNaTela(arvore); 
}

void funcionalidade15(){
    char letter1, letter2;
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

    FILE *linhaPointer = NULL;
    char linha[32];

    scanf("%s", linha);
    char temp[9];
    scanf("%s", temp);
    scanf("%s", temp);

    linhaPointer = fopen(linha, "rb");

    // Testes de falha de processamento
    if(linhaPointer == NULL){
        printf("Falha no processamento do arquivo.\n");
        free(v);
        fclose(veiculoPointer);
        return;
    }
    fread(&letter1, sizeof(char), 1, veiculoPointer);
    fread(&letter2, sizeof(char), 1, linhaPointer);
    if((letter1 == '0') | (letter2 == '0')){
        printf("Falha no processamento do arquivo.\n");
        free(v);
        fclose(linhaPointer);
        fclose(veiculoPointer);
        return;
    }

    fseek(veiculoPointer, 175L, SEEK_SET);
    
    v->categoria = (char *)malloc(256*sizeof(char));
    v->modelo = (char *)malloc(256*sizeof(char));
    
    char houveMatch;
    while(fread(&letter1, sizeof(char), 1, veiculoPointer) != 0){    // Armazena em letter a primeira letra da linha para conferir se o cursor está no final
        houveMatch = '0';
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

            Linha *line = (Linha*)malloc(sizeof(Linha));
            line->nomeLinha = (char*)malloc(sizeof(char)*64);
            line->corLinha = (char*)malloc(sizeof(char)*64);

            fseek(linhaPointer, 82L, SEEK_SET);
            while(fread(&letter2, sizeof(char), 1, linhaPointer) != 0){    // Armazena em letter a primeira letra da linha para conferir se o cursor está no final
                line->removido = letter2;   // O primeiro byte já é o "removido"
                fread(&line->tamanhoRegistro, sizeof(int), 1, linhaPointer);

                if(letter2 == '0') fseek(linhaPointer, line->tamanhoRegistro, SEEK_CUR);    // Caso o arquivo esteja logicamente removido, pula ele
                else{
                    fread(&line->codLinha, sizeof(int), 1, linhaPointer);

                    if(v->codLinha == line->codLinha){
                        fread(&line->aceitaCartao, sizeof(char), 1, linhaPointer);
                        fread(&line->tamanhoNome, sizeof(int), 1, linhaPointer);

                        // Caso o tamanho do nome seja 0, logo, o nome é nulo
                        if(line->tamanhoNome != 0){
                            fread(line->nomeLinha, sizeof(char), line->tamanhoNome, linhaPointer);
                        }
                        line->nomeLinha[line->tamanhoNome] = '\0';

                        fread(&line->tamanhoCor, sizeof(int), 1, linhaPointer);
                        fread(line->corLinha, sizeof(char), line->tamanhoCor, linhaPointer);
                        line->corLinha[line->tamanhoCor] = '\0';

                        houveMatch = '1';   // Se entrou no if, houve match
                    }else fseek(linhaPointer, line->tamanhoRegistro - sizeof(line->codLinha), SEEK_CUR);    // Caso os codLinha não sejam iguais, pula o resto do registro
                }

                if(houveMatch == '1'){
                    printVeiculoMatch(v);
                    printLinhaMatch(line);
                    break;
                }
            }
            free(line->nomeLinha);
            free(line->corLinha);
            free(line);
        }
    }

    if(houveMatch == '0') printf("Registro inexistente.\n");
    free(v->modelo);
    free(v->categoria);
    free(v);
    fclose(veiculoPointer);
    fclose(linhaPointer);
}

int funcionalidade17(char *veiculoDesordenadoNome, char *veiculoOrdenadoNome, char *campo){
    char letter;

    FILE *original = fopen(veiculoDesordenadoNome, "rb");

    // caso não haja arquivo, retorna erro
    if((original == NULL)){
        printf("Falha no processamento do arquivo.\n");
        return 0;
    }
    
    // Caso o arquivo esteja inconsistente, retorna erro
    fread(&letter, sizeof(char), 1, original);
    if(letter == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(original);
        return 0;
    }
    
    FILE *ordenado = fopen(veiculoOrdenadoNome, "wb+");

    fseek(original, 9L, SEEK_SET);
    int numberOfRegisters = 0;
    fread(&numberOfRegisters, sizeof(int), 1, original);
    
    int i = 0;
    int removidos = 0;
    Veiculo **v = (Veiculo**)malloc(numberOfRegisters * sizeof(Veiculo*));
    fseek(original, 175L, SEEK_SET);
    while(fread(&letter, sizeof(char), 1, original) != 0){    // Armazena em letter a primeira letra da linha para conferir se o cursor está no final
        v[i] = (Veiculo*)malloc(sizeof(Veiculo));
        fread(&v[i]->tamanhoRegistro, sizeof(int), 1, original);

        // Caso removido seja '0', já é pulado o registro
        if(letter == '0'){
            fseek(original, v[i]->tamanhoRegistro, SEEK_CUR);
            free(v[i]);
            i--;
            removidos++;
        }else{
            v[i]->removido = '1';
            fread(v[i]->prefixo, sizeof(char), 5, original);
            fread(v[i]->data, sizeof(char), 10, original);
            
            fread(&v[i]->quantidadeLugares, sizeof(int), 1, original);
            
            fread(&v[i]->codLinha, sizeof(int), 1, original);
            // Caso o tamanho do modelo seja 0, logo, o modelo é nulo
            fread(&v[i]->tamanhoModelo, sizeof(int), 1, original);
            if(v[i]->tamanhoModelo <= 0) v[i]->modelo = NULL;
            else{
                v[i]->modelo = (char *)malloc((v[i]->tamanhoModelo) * sizeof(char));
                fread(v[i]->modelo, sizeof(char), v[i]->tamanhoModelo, original);
            }

            // Segue o exemplo do registro acima
            fread(&v[i]->tamanhoCategoria, sizeof(int), 1, original);
            if(v[i]->tamanhoCategoria != 0){
                v[i]->categoria = (char*)malloc(sizeof(char)*v[i]->tamanhoCategoria);
                fread(v[i]->categoria, v[i]->tamanhoCategoria, 1, original);
            }else v[i]->categoria = NULL;
        }
        i++;
    }

    char cabecalho[175];
    fseek(original, 0L, SEEK_SET);
    fread(cabecalho, sizeof(char), 175, original);
    fwrite(cabecalho, sizeof(char), 175, ordenado);

    qsort(v, numberOfRegisters, sizeof(Veiculo*), (*compareVeiculos));

    for(int b = 0; b < numberOfRegisters; b++){
        escreveBinVeiculos(ordenado, v[b]);
    }

    for(int i = 0; i < numberOfRegisters; i++){
        free(v[i]->categoria);
        free(v[i]->modelo);
        free(v[i]);
    }
    free(v);

    Binary *bin = leCabecalho(original);
    bin->nroRegRemovidos = 0;
    bin->byteProxReg = ftell(ordenado);
    escreveCabecalhoVeiculos(ordenado, bin);

    free(bin);
    fclose(original);
    fclose(ordenado);

    return 1;
}

void funcionalidade19(){
    char veiculoDesordenado[32], linhaDesordenada[32], campo[16];
    int flag1 = 0, flag2 = 0;

    scanf("%s", veiculoDesordenado);
    scanf("%s", linhaDesordenada);
    scanf("%s", campo);
    scanf("%s", campo);

    flag1 = funcionalidade17(veiculoDesordenado, "veiculoOrdenado", campo);
    if(flag1 == 1) flag2 = funcionalidade18(linhaDesordenada, "linhaOrdenada", campo);  // Caso não tenha havido sucesso na primeira ordenação, nem perde tempo com a segunda

    // Caso alguma dos duas ordenações não tenha dado certo, returna
    // Não imprime nada pois nas próprias funcionalidades já é imprimido o erro
    if(flag1 == 0 || flag2 == 0) return;

    char letter1, letter2;
    Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));    // Struct usada para armazenar os dados de um veículo por vez

    FILE *veiculoPointer = fopen("veiculoOrdenado", "rb");  // Abre em modo de apenas leitura de arquivo binário
    FILE *linhaPointer = fopen("linhaOrdenada", "rb");  // Abre em modo de apenas leitura de arquivo binário

    // Testes de falha de processamento
    fread(&letter1, sizeof(char), 1, veiculoPointer);
    fread(&letter2, sizeof(char), 1, linhaPointer);

    // Caso algum arquivo esteja inconsistente, retorna erro
    if((letter1 == '0') | (letter2 == '0')){
        printf("Falha no processamento do arquivo.\n");
        free(v);
        fclose(linhaPointer);
        fclose(veiculoPointer);
        return;
    }

    fseek(veiculoPointer, 175L, SEEK_SET);  // Pula o cabeçalho de veiculos
    
    v->categoria = (char *)malloc(256*sizeof(char));
    v->modelo = (char *)malloc(256*sizeof(char));
    
    char houveMatch;
    while(fread(&letter1, sizeof(char), 1, veiculoPointer) != 0){    // Armazena em letter a primeira letra da linha para conferir se o cursor está no final
        houveMatch = '0';
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

            Linha *line = (Linha*)malloc(sizeof(Linha));
            line->nomeLinha = (char*)malloc(sizeof(char)*64);
            line->corLinha = (char*)malloc(sizeof(char)*64);

            fseek(linhaPointer, 82L, SEEK_SET);
            while(fread(&letter2, sizeof(char), 1, linhaPointer) != 0){    // Armazena em letter a primeira letra da linha para conferir se o cursor está no final
                line->removido = letter2;   // O primeiro byte já é o "removido"
                fread(&line->tamanhoRegistro, sizeof(int), 1, linhaPointer);

                if(letter2 == '0') fseek(linhaPointer, line->tamanhoRegistro, SEEK_CUR);
                else{
                    fread(&line->codLinha, sizeof(int), 1, linhaPointer);

                    if(v->codLinha == line->codLinha){
                        fread(&line->aceitaCartao, sizeof(char), 1, linhaPointer);
                        fread(&line->tamanhoNome, sizeof(int), 1, linhaPointer);

                        // Caso o tamanho do nome seja 0, logo, o nome é nulo
                        if(line->tamanhoNome != 0){
                            fread(line->nomeLinha, sizeof(char), line->tamanhoNome, linhaPointer);
                        }
                        line->nomeLinha[line->tamanhoNome] = '\0';

                        fread(&line->tamanhoCor, sizeof(int), 1, linhaPointer);
                        fread(line->corLinha, sizeof(char), line->tamanhoCor, linhaPointer);
                        line->corLinha[line->tamanhoCor] = '\0';

                        houveMatch = '1';   // Se entrou no if, houve match
                    }else fseek(linhaPointer, line->tamanhoRegistro - sizeof(line->codLinha), SEEK_CUR);
                }

                if(houveMatch == '1'){
                    printVeiculoMatch(v);
                    printLinhaMatch(line);
                    break;
                }
            }
            free(line->nomeLinha);
            free(line->corLinha);
            free(line);
        }
    }

    if(houveMatch == '0') printf("Registro inexistente.\n");
    free(v->modelo);
    free(v->categoria);
    free(v);
    fclose(veiculoPointer);
    fclose(linhaPointer);

}

int compareVeiculos(const void *a, const void *b){
    Veiculo *vA = *(Veiculo**)a;
    Veiculo *vB = *(Veiculo**)b;

    return (vA->codLinha - vB->codLinha);
}

void escreveBinVeiculos(FILE *binPointer, Veiculo *v){
    fseek(binPointer, 0L, SEEK_END);
    fwrite(&v->removido, sizeof(char), 1, binPointer);
    fwrite(&v->tamanhoRegistro, sizeof(int), 1, binPointer);
    fwrite(v->prefixo, sizeof(char), 5, binPointer);
    fwrite(v->data, sizeof(char), 10, binPointer);
    fwrite(&v->quantidadeLugares, sizeof(int), 1, binPointer);
    fwrite(&v->codLinha, sizeof(int), 1, binPointer);
    fwrite(&v->tamanhoModelo, sizeof(int), 1, binPointer);
    if(v->tamanhoModelo > 0) fwrite(v->modelo, sizeof(char), v->tamanhoModelo, binPointer);	// Apenas escreve caso não seja nulo
    fwrite(&v->tamanhoCategoria, sizeof(int), 1, binPointer);
    if(v->tamanhoCategoria > 0) fwrite(v->categoria, sizeof(char), v->tamanhoCategoria, binPointer);	// Apenas escreve caso não seja nulo
}

char *converteData(char *data){
    char *ext_data = (char *)calloc(50, sizeof(char));
    
    if(data[0] == '\0'){
        strcpy(ext_data, "campo com valor nulo");
        return ext_data;
    }
    // Passa o dia para a string
    ext_data[0] = data[8];
    ext_data[1] = data[9];

    strcat(ext_data, " de ");

    // Dependendo do mês, uma string diferente será concatenada
    if(data[5] == '0'){
        switch(data[6]){
            case '1':
                strcat(ext_data, "janeiro de ");
            break;
            
            case '2':
                strcat(ext_data, "fevereiro de ");
            break;

            case '3':
                strcat(ext_data, "março de ");
            break;

            case '4':
                strcat(ext_data, "abril de ");
            break;

            case '5':
                strcat(ext_data, "maio de ");
            break;

            case '6':
                strcat(ext_data, "junho de ");
            break;

            case '7':
                strcat(ext_data, "julho de ");
            break;

            case '8':
                strcat(ext_data, "agosto de ");

            break;
            case '9':
                strcat(ext_data, "setembro de ");
            break;
        }
    }else{
        switch(data[6]){
            case '0':
                strcat(ext_data, "outubro de ");
            break;

            case '1':
                strcat(ext_data, "novembro de ");
            break;
            
            case '2':
                strcat(ext_data, "dezembro de ");
            break;
        }
    }
    
    strncat(ext_data, data, 4); // Concatena o ano
    ext_data[strlen(ext_data)] = 0;

    return ext_data;
}

void printVeiculoMatch(Veiculo *vehicle){
    char *new_data = converteData(vehicle->data);

    printf("Prefixo do veiculo: %s\n", vehicle->prefixo);
    printf("Modelo do veiculo: %s\n", vehicle->tamanhoModelo != 0 ? vehicle->modelo : "campo com valor nulo");
    printf("Categoria do veiculo: %s\n", vehicle->tamanhoCategoria != 0 ? vehicle->categoria : "campo com valor nulo");
    printf("Data de entrada do veiculo na frota: %s\n", new_data);
    printf("Quantidade de lugares sentados disponiveis: %d\n", vehicle->quantidadeLugares);

    free(new_data);
}
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include"file.h"
#include"arvore-b.h"

void binarioNaTela(char *nomeArquivoBinario) { /* Você não precisa entender o código dessa função. */

	/* Use essa função para comparação no run.codes. Lembre-se de ter fechado (fclose) o arquivo anteriormente.
	*  Ela vai abrir de novo para leitura e depois fechar (você não vai perder pontos por isso se usar ela). */

	unsigned long i, cs;
	unsigned char *mb;
	size_t fl;
	FILE *fs;
	if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
		fprintf(stderr, "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): não foi possível abrir o arquivo que me passou para leitura. Ele existe e você tá passando o nome certo? Você lembrou de fechar ele com fclose depois de usar?\n");
		return;
	}
	fseek(fs, 0, SEEK_END);
	fl = ftell(fs);
	fseek(fs, 0, SEEK_SET);
	mb = (unsigned char *) malloc(fl);
	fread(mb, 1, fl, fs);

	cs = 0;
	for(i = 0; i < fl; i++) {
		cs += (unsigned long) mb[i];
	}
	printf("%lf\n", (cs / (double) 100));
	free(mb);
	fclose(fs);
}

void scan_quote_string(char *str) {

	/*
	*	Use essa função para ler um campo string delimitado entre aspas (").
	*	Chame ela na hora que for ler tal campo. Por exemplo:
	*
	*	A entrada está da seguinte forma:
	*		nomeDoCampo "MARIA DA SILVA"
	*
	*	Para ler isso para as strings já alocadas str1 e str2 do seu programa, você faz:
	*		scanf("%s", str1); // Vai salvar nomeDoCampo em str1
	*		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2 (sem as aspas)
	*
	*/

	char R;

	while((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

	if(R == 'N' || R == 'n') { // campo NULO
		getchar(); getchar(); getchar(); // ignorar o "ULO" de NULO.
		strcpy(str, ""); // copia string vazia
	} else if(R == '\"') {
		if(scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
			strcpy(str, "");
		}
		getchar(); // ignorar aspas fechando
	} else if(R != EOF){ // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então, pois deve ser algum inteiro ou algo assim...
		str[0] = R;
		scanf("%s", &str[1]);
	} else { // EOF
		strcpy(str, "");
	}
}

/*  
    Converte o prefixo do veículo para int

    OBS1:   retorna -1 se o primeiro caracter é '*'

    OBS2:   retorna LIXO se a string contém algo diferente de números e letras 
            maiúsculas (com excessão do caso da OBS1)

    COMO FUNCIONA:

        Para converter os prefixos para int e garantir que prefixos diferentes 
        deem números diferentes interpretamos eles como números em base 36

        Um número N com 5 digitos dI em base 36 é escrito como:

            N = d0 * 36^0 + d1 * 36^1 + d2 * 36^2 + d3 * 36^3 + d4 * 36^4

        Nota-se que estamos escrevendo do digito menos significativo para o 
        mais significativo

        Como o nosso prefixo têm 5 bytes e eles podem assumir 36 valores
        cada, podemos interpretar cada char dele como um digito em base 36, 
        prefixos diferentes formaram números diferentes em base 36 e portanto 
        números diferentes quando convertemos para um int normal
*/
int convertePrefixo(char* str) {
    /* O registro que tem essa string como chave foi removido */
    if(str[0] == '*')
        return -1;

    /* Começamos com o primeiro digito na ordem de 36^0 = 1 */
    int power = 1;

    /* Faz a conversão char por char para chegar ao resultado */
    int result = 0;
    for(int i = 0; i < 5; i++) {

        /* 
            Interpreta o char atual como se fosse um digito
            em base 36. Os digitos da base 36 são:
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D,
            E, F, G, H, I, J, K, L, M, N, O, P, Q, R,
            S, T, U, V, W, X, Y, Z
        */
        int cur_digit;
        /* Checa pelos digitos normais e os converte para números */
        if(str[i] >= '0' && str[i] <= '9')
            cur_digit = str[i] - '0';
        /* Checa pelas letras e as converte para números */
        else if(str[i] >= 'A' && str[i] <= 'Z')
            cur_digit = 10 + str[i] - 'A';

        /*
            Multiplica o digito atual pelo ordem da posição atual
            e adiciona no resultado
            Primeira posição:   36^0 = 1
            Segunda posição:    36^1 = 36
            Terceira posição:   36^2 = 1.296
            Quarta posição:     36^3 = 46.656
            Quinta posição:     36^4 = 1.679.616
        */
        result += cur_digit * power;

        /* Aumenta a ordem atual */
        power *= 36;
    }

    return result;
}

char *freadline(FILE *stream, char *line){
	char letter = 0;
	int pos = 0;	// Deixa o contador com 0, inicialmente

	do{
		letter = fgetc(stream);
        if(letter != ',' && letter != '\n' && letter != EOF) line[pos++] = letter;	// Caso não seja ',', '\n' ou EOF, escreve na string e incrementa o tamanho
        else break;
    }while(1);

    if(!strncmp(line, "NULO", 4)){	// Caso o campo seja nulo, retorna tamanho 0 e uma string vazia
        line[0] = '\0';
        return line;
    }
	
	line[pos] = '\0';

	return line;
}

void escreveCabecalhoVeiculos(FILE *binPointer, Binary *b){
	fseek(binPointer, 0L, SEEK_SET);	// Vai para o começo do arquivo (cabeçalho)

    fwrite(&b->status, sizeof(char), 1, binPointer);
    fwrite(&b->byteProxReg, sizeof(long long int), 1, binPointer);
    fwrite(&b->nroRegistros, sizeof(int), 1, binPointer);
    fwrite(&b->nroRegRemovidos, sizeof(int), 1, binPointer);
    fwrite("Prefixo do veiculo", sizeof(char), 18, binPointer);
    fwrite("Data de entrada do veiculo na frota", sizeof(char), 35, binPointer);
    fwrite("Quantidade de lugares sentados disponiveis", sizeof(char), 42, binPointer);
    fwrite("Linha associada ao veiculo", sizeof(char), 26, binPointer);
    fwrite("Modelo do veiculo", sizeof(char), 17, binPointer);
    fwrite("Categoria do veiculo", sizeof(char), 20, binPointer);
}

Binary *leCabecalho(FILE *binPointer){
    fseek(binPointer, 0L, SEEK_SET);

    Binary *bin = (Binary*)malloc(sizeof(Binary));
    fread(&bin->status, sizeof(char), 1, binPointer);
    fread(&bin->byteProxReg, sizeof(long long int), 1, binPointer);
    fread(&bin->nroRegistros, sizeof(int), 1, binPointer);
    fread(&bin->nroRegRemovidos, sizeof(int), 1, binPointer);

    return bin;
}

void escreveCabecalhoLinhas(FILE *binPointer, Binary *bin){
	fseek(binPointer, 0L, SEEK_SET);

    fwrite(&bin->status, sizeof(char), 1, binPointer);
    fwrite(&bin->byteProxReg, sizeof(long long int), 1, binPointer);
    fwrite(&bin->nroRegistros, sizeof(int), 1, binPointer);
    fwrite(&bin->nroRegRemovidos, sizeof(int), 1, binPointer);
}

void copiaCabecalhoLinhas(FILE *csvPointer, FILE *binPointer, Binary *bin){
    fseek(binPointer, 0L, SEEK_SET);
    fseek(csvPointer, 0L, SEEK_SET);

    fwrite(&bin->status, sizeof(char), 1, binPointer);
    fwrite(&bin->byteProxReg, sizeof(long long int), 1, binPointer);
    fwrite(&bin->nroRegistros, sizeof(int), 1, binPointer);
    fwrite(&bin->nroRegRemovidos, sizeof(int), 1, binPointer);
    
    char *cabecalhoBuffer = (char*)malloc(sizeof(char)*24);
    fread(cabecalhoBuffer, sizeof(char), 15, csvPointer);
    fwrite(cabecalhoBuffer, sizeof(char), 15, binPointer);
    fread(cabecalhoBuffer, sizeof(char), 1, csvPointer);


    fread(cabecalhoBuffer, sizeof(char), 13, csvPointer);
    fwrite(cabecalhoBuffer, sizeof(char), 13, binPointer);
    fread(cabecalhoBuffer, sizeof(char), 1, csvPointer);

    fread(cabecalhoBuffer, sizeof(char), 13, csvPointer);
    fwrite(cabecalhoBuffer, sizeof(char), 13, binPointer);
    fread(cabecalhoBuffer, sizeof(char), 1, csvPointer);

    fread(cabecalhoBuffer, sizeof(char), 24, csvPointer);
    fwrite(cabecalhoBuffer, sizeof(char), 24, binPointer);

    free(cabecalhoBuffer);
}

// Salva as descrições do cabeçalho em strings diferentes
void salvaCabecalho(FILE *binPointer, char **cab1, char **cab2, char **cab3, char **cab4, char **cab5){
	fread(*cab1, sizeof(char), 18, binPointer);
	(*cab1)[18] = 0;

	fread(*cab2, sizeof(char), 35, binPointer);
	(*cab2)[35] = 0;

	fread(*cab3, sizeof(char), 42, binPointer);
	(*cab3)[42] = 0;

	fseek(binPointer, 26, SEEK_CUR);

	fread(*cab4, sizeof(char), 17, binPointer);
	(*cab4)[17] = 0;

	fread(*cab5, sizeof(char), 20, binPointer);
	(*cab5)[20] = 0;
}
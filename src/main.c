//TRABALHO PRÁTICO 3
//Joao Francisco Caprioli Barbosa Camargo de Pinho//ID USP: 10748500
//Gustavo Lelli Guirao//ID USP: 11918182
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linhas.h>
#include <file.h>
#include <veiculos.h>

void operate(int operation){
    char *arquivoDesordenado, *arquivoOrdenado, *campo;
    int flag;

    //essa função operate apenas usa o número enviado pelo usuário para direcionar o programa
    switch(operation){
        //Trabalho 1 VVVVV
        case 1:
            funcionalidade1();
            break;

        case 2:
            funcionalidade2();
            break;

        case 3:
            funcionalidade3();
            break;

        case 4:
            funcionalidade4();
            break;

        case 5:
            funcionalidade5();
            break;

        case 6:
            funcionalidade6();
            break;

        case 7:
            funcionalidade7();
            break;

        case 8:
            funcionalidade8();
            break;

        //Trabalho 2 VVVVV
        case 9:
            funcionalidade9();
            break;

        case 10:
            funcionalidade10();
            break;

        case 11:
            funcionalidade11();
            break;

        case 12:
            funcionalidade12();
            break;

        case 13:
            funcionalidade13();
            break;

        case 14:
            funcionalidade14();
            break;

        // Trabalho 3 VVVVV
        case 15:
            funcionalidade15();
            break;

        case 16:
            funcionalidade16();
            break;

        case 19:
            funcionalidade19();
            break;
    
        default:
            arquivoDesordenado = (char *)malloc(32 * sizeof(char));
            arquivoOrdenado = (char *)malloc(32 * sizeof(char));
            campo = (char *)malloc(16 * sizeof(char));
            
            scanf("%s", arquivoDesordenado);
            scanf("%s", arquivoOrdenado);
            scanf("%s", campo);
 
            if(operation == 17) flag = funcionalidade17(arquivoDesordenado, arquivoOrdenado, campo);
            else if(operation == 18) flag = funcionalidade18(arquivoDesordenado, arquivoOrdenado, campo);
            
            if(flag == 1) binarioNaTela(arquivoOrdenado);
            free(arquivoOrdenado);
            free(arquivoDesordenado);
            free(campo);
    }
}

int main(){
    //a função main apenas recebe o número da operação e manda para a função operate
    int operation;
    scanf("%d", &operation);
    operate(operation);
}
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <ctime>

#include "ils.h"
#include "vnd.h"

using namespace std;

void perturbar(Airport* airport, size_t tamanhoBloco = 2) {

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> pistaDist(0, airport->num_pistas - 1);

    int pistaA, pistaB;
    do {
        pistaA = pistaDist(gen);
        pistaB = pistaDist(gen);
    } while (pistaA == pistaB || 
             airport->pistas[pistaA].size() < tamanhoBloco || 
             airport->pistas[pistaB].size() < tamanhoBloco);

    uniform_int_distribution<> idxA(0, airport->pistas[pistaA].size() - tamanhoBloco);
    uniform_int_distribution<> idxB(0, airport->pistas[pistaB].size() - tamanhoBloco);

    int startA = idxA(gen);
    int startB = idxB(gen);    

    // Troca os blocos de voos entre as pistas
    vector<int> blocoA(airport->pistas[pistaA].begin() + startA, 
                      airport->pistas[pistaA].begin() + startA + tamanhoBloco);
    vector<int> blocoB(airport->pistas[pistaB].begin() + startB, 
                      airport->pistas[pistaB].begin() + startB + tamanhoBloco);

    // Realiza a troca
    for (size_t i = 0; i < tamanhoBloco; i++) {
        airport->pistas[pistaA][startA + i] = blocoB[i];
        airport->pistas[pistaB][startB + i] = blocoA[i];
        
        // Atualiza as pistas alocadas nos objetos Voo
        Voo* vooB = airport->encontrarVooPorId(blocoB[i]);
        Voo* vooA = airport->encontrarVooPorId(blocoA[i]);
        
        if (vooB) vooB->pista_alocada = pistaA;
        if (vooA) vooA->pista_alocada = pistaB;
    }    

    // Atualiza horários e predecessores
    auto atualizarPista = [airport](int pista) {
        int anterior = -1; // ID do voo anterior
        
        for (size_t i = 0; i < airport->pistas[pista].size(); i++) {
            int id_atual = airport->pistas[pista][i];
            Voo* voo_atual = airport->encontrarVooPorId(id_atual);
            
            if (!voo_atual) {
                continue;
            }
            
            // Atualiza voo anterior
            voo_atual->voo_anterior = anterior;
            
            // Calcula horário real
            if (anterior == -1) {
                voo_atual->horario_real = voo_atual->horario_prev;
            } else {
                Voo* voo_anterior = airport->encontrarVooPorId(anterior);
                if (voo_anterior) {
                    int espera = airport->tempo_espera[anterior][id_atual];
                    voo_atual->horario_real = max(
                        voo_atual->horario_prev,
                        voo_anterior->horario_real + voo_anterior->duracao + espera
                    );
                }
            }
            
            anterior = id_atual;
        }
    };
    

    atualizarPista(pistaA);
    atualizarPista(pistaB);

    airport->calcularMultas();

}

void ILS(Airport* airport, int maxIter) {

    // Executa VND inicial
    VND(airport);
    airport->salvarMelhorSolucao();
    int melhorCusto = airport->bkp_custo;

    for (int iter = 0; iter < maxIter; iter++) {
        // Aplica perturbação
        perturbar(airport);

        // Refina com VND
        VND(airport);

        // Avalia a solução
        int custoAtual = airport->custo_total;
        if (custoAtual < melhorCusto) {
            *airport = *airport;  // airport a solução melhorada
            airport->salvarMelhorSolucao();
            melhorCusto = custoAtual;
        }

    }
    // Restaura a melhor solução encontrada
    airport->restaurarMelhorSolucao();
}
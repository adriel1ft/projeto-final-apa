#ifndef ALOCADOR_H
#define ALOCADOR_H

#include <string>
using namespace std;

bool executarAlocacao(const string& arquivo_entrada);
bool inverterVoosConsecutivos(int pista, int posicao_voo);

extern int custo_total;

#endif //ALOCADOR_H

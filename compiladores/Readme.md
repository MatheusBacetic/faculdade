PROJETO COMPILADORES - FASE 1 (Analisador Léxico e Sintático)
Aluno: Matheus Veiga Bacetic Joaquim

1. STATUS DO DESENVOLVIMENTO
As Etapas 1, 2 e 3 foram concluídas com sucesso. O compilador é capaz de realizar a varredura léxica gerando o arquivo "saida_lexica.txt" e realizar a análise sintática descendente recursiva conforme a gramática definida, abortando a execução e exibindo a linha correta ao encontrar erros.

2. COMO COMPILAR
Utilize o compilador MinGW no terminal com o seguinte comando:
gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador

3. COMO EXECUTAR
Passe o arquivo fonte MiniPython como argumento de linha de comando:
./compilador programa.txt

4. DECISÕES DE DESIGN E IMPLEMENTAÇÃO
- O analisador léxico trata as palavras reservadas (if, while, True, etc.) em uma função auxiliar `classificarLexema()` que utiliza `strcmp`, simplificando as transições do autômato finito.
- A função `obter_atomo()` utiliza a estratégia de lookahead (espiar o próximo caractere com `fgetc` e devolver com `ungetc`) para resolver ambiguidades em operadores compostos como '==' e '**'.
- A análise sintática foi construída com funções mutuamente recursivas (programa, comando, expressao, fator) baseadas no token `lookahead`.

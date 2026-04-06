===================================================================
PROJETO COMPILADORES - FASE 1 (Análise Léxica e Sintática)
Alunos:
- Matheus Veiga Bacetic Joaquim (RA: 10425638)
- Beatriz Barbosa (RA: 10354067)
- Gabriel Pereira Faravola (RA: 10427189)
===================================================================

1. STATUS DO DESENVOLVIMENTO
As Etapas 1, 2 e 3 foram entregues e estão funcionais. O analisador léxico consegue varrer o código e gerar o arquivo "saida_lexica.txt" corretamente. O analisador sintático (descendente recursivo) está validando a gramática do MiniPython. Caso encontre erros léxicos ou sintáticos, o programa aborta a execução imediatamente, indicando a linha e o token problemático, conforme a especificação.

2. COMO COMPILAR
O projeto foi desenvolvido em C. Para compilar no MinGW sem gerar warnings, utilize o comando abaixo no terminal:

gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador

3. COMO EXECUTAR
Após a compilação, execute passando o arquivo fonte como argumento:

- No Linux/Codespaces: ./compilador programa.py
- No prompt do Windows: .\compilador.exe programa.py

4. DECISÕES DE DESIGN E IMPLEMENTAÇÃO
- Interface Léxico/Sintático: Para respeitar a restrição do projeto de usar exatamente a função `obter_atomo()`, criamos uma função interna (`motor_lexico()`) que faz a leitura bruta dos caracteres. A função `obter_atomo()` atua como um wrapper que chama esse motor, grava o token no arquivo .txt e o devolve ao sintático, mantendo o código organizado.
- Lookahead: Utilizamos a função nativa `ungetc` do C para "espiar" o próximo caractere e resolver ambiguidades de operadores compostos (como diferenciar '=' de '=='), devolvendo o caractere ao buffer quando necessário.
- Precedência na Sintaxe: A gramática foi dividida hierarquicamente (expressao > expRelacional > expSimples > termo > fator) para garantir que a árvore de análise resolva as operações na ordem correta (primeiro matemática, depois relacionais e lógicos).
- Filtro de Palavras Reservadas: O analisador léxico captura sequências de caracteres como identificadores e as repassa para a função auxiliar `classificarLexema()`. Essa função usa `strcmp` para separar identificadores comuns de palavras reservadas, simplificando as transições do autômato principal.

5. BUGS CONHECIDOS
Não há bugs lógicos identificados na implementação atual. O compilador foi testado com estruturas aninhadas, acesso a listas e erros intencionais de sintaxe, comportando-se conforme o esperado para a linguagem MiniPython.
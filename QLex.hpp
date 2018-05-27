#ifndef QLEX_HPP
#define QLEX_HPP

/***********************************************************
From: Marco Qiaofeng Liu (Email: qiaofengmarco@outlook.com),
      School of Computer Science and Engineering,
      Southeast University, Jiulonghu Campus,
      Nanjing, China
************************************************************/

#include "Headers.h"
#include "DFA.hpp"
#include "Preprocess.hpp"

class QLex
{
public:
    QLex() {}
    void LexCreate(string FileName)
    {
        Preprocessor Pre(FileName);
        vector<string> str;
        Pre.getResult(str);
        vector<DFA> dfas;
        DFA result_dfa;
        for (unsigned int i = 0; i < str.size(); i++)
        {
            cout << i << endl;
            dfas.push_back(DFA(str[i]));
        }
        merge(result_dfa, dfas);
        result_dfa.minimize();
        TableToCode();
        RemainingCodeGen();
        cout << "Finished all code generations!" << endl;
        cout << "The Lexical Analyzer Code is in lex.yy.c." << endl;
    }
private:
    void RemainingCodeGen()
    {
        ofstream out("./lex.yy.c", ios::app);
        cout << "Generating Remaining Code..." << endl;
        out << "unsigned int max_capacity = 5000;" << endl;
        out << "char *temp;" << endl;
        out << "void cat(char **a, const char b) {" << endl;
        out << "    if (max_capacity < yyleng + 2) {" << endl;
        out << "        max_capacity += 1000;" << endl;
        out << "        if (temp != NULL) {" << endl;
        out << "            free(temp);" << endl;
        out << "            temp = NULL;" << endl;
        out << "        }" << endl;
        out << "        temp = (char*) malloc(sizeof(char) * max_capacity);" << endl;
        out << "        strcpy(temp, *a);" << endl;
        out << "        free(*a);" << endl;
        out << "        *a = temp;" << endl;
        out << "    }" << endl;
        out << "    (*a)[yyleng++] = b;" << endl;
        out << "    (*a)[yyleng] = \'\\0\';" << endl;
        out << "}" << endl;
        out << "int main(int argc, char *argv[]) {" << endl;
        out << "    int pid = 0, token_id, cur_state = 0, ending;" << endl;
        out << "    char next;" << endl;
        out << "    if ((argc < 2) || (argc > 3)) {" << endl;
        out << "        printf(\"Parameters Error.\\n\");" << endl;
        out << "        return 1;" << endl;
        out << "    }" << endl;
        out << "    else if (argc == 2)" << endl;
        out << "        yyout = fopen(\"./Result.txt\", \"w\");" << endl;
        out << "    else" << endl;
        out << "        yyout = fopen(argv[2], \"w\");" << endl;
        out << "    yyin = fopen(argv[1], \"r\");" << endl;
        out << "    yytext = (char*) calloc(max_capacity, sizeof(char));" << endl;
        out << "    yytext[0] = \'\\0\'; " << endl;
        out << "    yyleng = 0;" << endl;
        out << "    next = fgetc(yyin);" << endl;
        out << "    fprintf(yyout, \"Line   1:\");" << endl;
        out << "    while (next != EOF) {" << endl;
        out << "        if (table[cur_state][next] >= 0) {" << endl;
        out << "            cur_state = table[cur_state][next];" << endl;
        out << "            next = fgetc(yyin); " << endl;
        out << "        }" << endl;
        out << "        else {" << endl;
        out << "            if (yyleng >= 1)" << endl;
        out << "                yytext[--yyleng] = \'\\0\';" << endl;
        out << "            ending = table[cur_state][(int)'$'];" << endl;
        out << "            if ((ending >= 0) && (acc_table[ending] >= 0)) {" << endl;
        out << "                pid = acc_table[ending];" << endl;
        out << "                token_id = act(pid);" << endl;
        out << "                if (token_id > 0)" << endl;
        out << "                    fprintf(yyout, \"    <%d, %s>\", token_id, yytext);" << endl;
        out << "                else if (token_id == -1) {" << endl;
        out << "                    printf(\"An error has occurred.\\n\");" << endl;
        out << "                    return 1;" << endl;
        out << "                }" << endl;
        out << "                yyleng = 0;" << endl;
        out << "                yytext[0] = \'\\0\';" << endl;
        out << "                cur_state = 0;" << endl;
        out << "            }" << endl;
        out << "        }" << endl;
        out << "        cat(&yytext, next);" << endl;
        out << "    }" << endl;
        out << "    free(yytext);" << endl;
        out << "    yytext = NULL;" << endl;
        out << "    fclose(yyin);" << endl;
        out << "    fclose(yyout);" << endl;
        out << "    return 0;" << endl;
        out << "}" << endl;
        cout << "Finished Remaining Code Generations!" << endl;
        out.close();
    }
    void TableToCode()
    {
        ofstream out("./lex.yy.c", ios::app);
        cout << "Printing Transition Table..." << endl;
        out << "int state_num = " << DFA::state_num << ";" << endl;
        out << "int acc_num = " << DFA::terminated_num << ";" << endl;

        out << "int table[" << DFA::state_num << "]" << "[130] = {" << endl;
        for (int i = 0; i < DFA::state_num; i++)
        {
            out << "{";
            for (int act = 0; act < 130; act++)
            {
                if (DFA::states[i]->links[act].non_empty)
                    out << DFA::states[i]->links[act].To->state_id - DFA::states[0]->state_id;
                else
                    out << "-1";
                if (act < 129)
                    out << ", ";
            }
            out << "}";
            if (i < DFA::state_num - 1)
                out << ",";
            out << endl;
        }
        out << "};" << endl;

        out << "int acc_table[" << DFA::state_num << "] = {";
        for (int i = 0; i < DFA::state_num; i++)
        {
            if (DFA::states[i]->state_type >= 0)
                out << DFA::states[i]->state_type;
            else
                out << "-1";
            if (i < DFA::state_num - 1)
                out << ",";
        }
        out << "};" << endl;

        cout << "Finished printing table!" << endl;
        out.close();
    }
    //print the transition table
    static void printTable()
    {
        ofstream out("./Table.txt", ios::out);
        cout << "Printing Transition Table..." << endl;
        out << "Table" << endl;
        out << DFA::state_num << endl;
        for (int i = 0; i < DFA::state_num; i++)
        {
            //out << DFA::states[i]->state_id - DFA::states[0]->state_id << " ";
            for (int act = 0; act < 130; act++)
            {
                if (act > 0)
                    out << " ";
                if (DFA::states[i]->links[act].non_empty)
                    out << DFA::states[i]->links[act].To->state_id - DFA::states[0]->state_id;
                else
                    out << "-1";
            }
            out << endl;
        }
        out << "ACCEPT" << endl;
        out << DFA::terminated_num << endl;
        for (int i = 0; i < DFA::state_num; i++)
        {
            if (DFA::states[i]->state_type >= 0)
            {
                out << DFA::states[i]->state_id - DFA::states[0]->state_id << " " << DFA::states[i]->state_type << endl;
            }
        }
        cout << "Finished printing table!" << endl;
        out.close();
    }
};

#endif
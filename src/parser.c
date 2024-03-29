#include "parser.h"

int result;			// globalni promenna pro kontrolu chyb
sParams paramsTmp = NULL;	// globalni promenna ve ktere uchovavam odkaz na parametr


int checkFunctions(symbolTablePtr *symbolTable){			// funkce ktera projde tabulkou symbolu a zjisti, zda se tam nenachazi funkce ktera byla deklarovana, ale nebyla definovana
	symbolTablePtr tmp;
	tmp = *symbolTable;

	if (tmp == NULL){
		return SYNTAX_OK;
	}

	if (tmp->content.isFunction){
		if (tmp->content.isDefined == false){
			return SEM_ERR;
		}
	}

	if (tmp->LPtr != NULL){
		return checkFunctions(&(*symbolTable)->LPtr);
	}
	if (tmp->RPtr != NULL){
		return checkFunctions(&(*symbolTable)->RPtr);
	}

	return SYNTAX_OK;
}

int statement(tToken *token, symbolTablePtr *symbolTable){

}


int body(tToken *token, symbolTablePtr *symbolTable){

	if (token->state != T_KEYWORD){
		return SYNTAX_ERR;
	}

	if (strcmp(token->content, "begin\0") != 0){
		return SYNTAX_ERR;
	}

	/**token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	result = precedence(token);*/
	// ZDE SE ZAVOLA FUNKCE PRO ANALYZU HLAVNIHO TELA PROGRAMU


	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}
	if (token->state != T_KEYWORD){
		return SYNTAX_ERR;
	}

	if (strcmp(token->content, "end\0") != 0){
		return SYNTAX_ERR;
	}

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}
	if (token->state != T_DOT){
		return SYNTAX_ERR;
	}

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state != T_EOF){
		return SYNTAX_ERR;
	}


	return result;
}

int funcStatement(tToken *token, symbolTablePtr *symbolTable){
	if (token->state != T_KEYWORD){
		return SYNTAX_ERR;

	}
	if (strcmp(token->content, "begin\0") != 0){
		return SYNTAX_ERR;

	}

	// ZDE SE ZAVOLA FUNKCE PRO ANALYZU TELA FUNKCE

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state != T_KEYWORD){
		return SYNTAX_ERR;
		
	}
	if (strcmp(token->content, "end\0") != 0){
		return SYNTAX_ERR;

	}

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state != T_SEMICOLON){
		return SYNTAX_ERR;
	}

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}
	if (token->state != T_KEYWORD){
		return SYNTAX_ERR;
	}

	return result;
}

int funcVarList(tToken *token, symbolTablePtr *symbolTable, symbol *sym, symbolTablePtr test){
	if (token->state != T_IDENTIFICATOR){
		return SYNTAX_ERR;
	}

	symbol tmp;
	tmp.key = token->content;
	tmp.isFunction = false;
	tmp.isDefined = false;
	tmp.params = NULL;
	tmp.symbolTable = NULL;

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state != T_COLON){
		return SYNTAX_ERR;
	}

	result = varType(token, symbolTable, &tmp);
	if (result != SYNTAX_OK)
		return result;

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}
	if (token->state != T_SEMICOLON){
		return SYNTAX_ERR;
	}


	// kontrola nazvu promennych
	if (!strcmp(sym->key, tmp.key)){
		result = SEM_ERR;
		return result;
	}

	if (test == NULL){
		sParams tmp2 = sym->params;
		while (tmp2 != NULL){
			if (!strcmp(tmp2->param.name, tmp.key)){
				result = SEM_ERR;
				return result;
			}
			tmp2 = tmp2->ptr;
		}
		if (BTInsert(&tmp.symbolTable, tmp.key, tmp) == BT_ERR){		// pokud funkce je deklarovana a definovana soucasne
			result = SEM_ERR;
			return result;
		}
	}
	else{
		sParams tmp2 = test->content.params;
		while (tmp2 != NULL){
			if (!strcmp(tmp2->param.name, tmp.key)){
				result = SEM_ERR;
				return result;
			}
			tmp2 = tmp2->ptr;
		}

		if (BTInsert(&test->content.symbolTable, tmp.key, tmp) == BT_ERR){	// pokud funkce jiz byla deklarovana ukladam do jiz existujici instance tabulky symbolu
			result = SEM_ERR;
			return result;
		}
	}

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state == T_IDENTIFICATOR){
		result = funcVarList(token, symbolTable, sym, test);
		if (result != SYNTAX_OK)
			return result;
	}
	else{
		return result;
	}



	return result;
}

int funcVariables(tToken *token, symbolTablePtr *symbolTable, symbol *sym, symbolTablePtr test){
	if (token->state == T_KEYWORD){
		if (!strcmp(token->content, "var\0")){
			*token = tGetToken();
			if (token->state == T_ERR){
				return LEX_ERR;
			}
			result = funcVarList(token, symbolTable, sym, test);
			if (result != SYNTAX_OK)
				return result;
		}
		
	}

	return result;
}

int params(tToken *token, symbolTablePtr *symbolTable, symbol *sym, symbolTablePtr test){
	if (token->state == T_RC){
		if (test != NULL){
			if (test->content.params != NULL && sym->params == NULL){			// kontrola prazdnych argumentu
				return SEM_ERR;
			}
		}
		paramsTmp = NULL;
		return result;
	}

	sParam param;

	if (token->state != T_IDENTIFICATOR){
		return SYNTAX_ERR;
	}

	param.name = token->content;

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state != T_COLON){
		return SYNTAX_ERR;
	}

	symbol tmp;	
	result = varType(token, symbolTable, &tmp);
	if (result != SYNTAX_OK)
		return result;
	param.type = tmp.type;

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}
	// ukladani parametru do struktury

	if (test == NULL){

		if (!strcmp(sym->key, param.name)){				// pokud nazev parametru je stejny jako identifikator funkce -> chyba
			return SEM_ERR;
		}
		if (sym->params == NULL){
			if ((sym->params = gMalloc(sizeof(struct params))) == NULL){
				result = INT_ERR;
				return result;
			}
			sym->params->param = param;
			sym->params->ptr = NULL;
			paramsTmp = sym->params;
		}
		else{
			if ((paramsTmp->ptr = gMalloc(sizeof(struct params))) == NULL){
				result = INT_ERR;
				return result;
			}
			paramsTmp->ptr->param = param;
			paramsTmp->ptr->ptr = NULL;
			paramsTmp = paramsTmp->ptr;

			sParams tmp = sym->params;								// pomocna promenna

			while (tmp != NULL){
				if (!strcmp(tmp->param.name, param.name)){			// zkontroluji seznam parametru, zda se tam jiz nenachazi
					return SEM_ERR;
				}
				tmp = tmp->ptr;
			}
		}
	}// kontrola parametru
	else{
		if (paramsTmp == NULL){			// snazim se porovnavat neco s necim co neexistuje -> error
			return SEM_ERR;
		}

		if (!strcmp(sym->key, param.name)){				// pokud nazev parametru je stejny jako identifikator funkce -> chyba
			return SEM_ERR;
		}

		if ((strcmp(param.name, paramsTmp->param.name)) != 0){
			return SEM_ERR;
		}
		if (param.type != paramsTmp->param.type){
			return SEM_ERR;
		}
		paramsTmp = paramsTmp->ptr;

	}

	if (token->state == T_SEMICOLON){
		*token = tGetToken();
		if (token->state == T_ERR){
			return LEX_ERR;
		}
		result = params(token, symbolTable, sym, test);
		if (result != SYNTAX_OK)
			return result;
	}
	if (token->state == T_RC){
		return result;
	}
	else{
		return SYNTAX_ERR;
	}


	return result;
}

int defFunction(tToken *token, symbolTablePtr *symbolTable){
	if (token->state != T_KEYWORD){
		return result;
	}

	if (strcmp(token->content, "function\0") != 0){
		return result;
	}

	symbol tmp;

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state != T_IDENTIFICATOR){
		return LEX_ERR;
	}

	symbolTablePtr test = NULL;

	if((test = BTSearch(symbolTable, token->content)) != NULL){
		if (test->content.isDefined){
			return SEM_ERR;
		}
	}


	tmp.key = token->content;
	tmp.isFunction = true;
	tmp.isDefined = false;
	tmp.params = NULL;
	tmp.symbolTable = NULL;
	

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state != T_LC){
		return LEX_ERR;
	}

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (test != NULL)
		paramsTmp = test->content.params;				// pokud funkce jiz byla deklarovana, musim zkontrolovat parametry

	result = params(token, symbolTable, &tmp, test);
	if (result != SYNTAX_OK)
		return result;
	paramsTmp = NULL;									// globalni promennou opet inicializuji na NULL

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state != T_COLON){
		return SYNTAX_ERR;
	}

	result = varType(token, symbolTable, &tmp);
	if (result != SYNTAX_OK)
		return result;

	if (test != NULL){							// kontrola navratove hodnoty funkce
		if (test->content.type != tmp.type){
			return SEM_ERR;
		}
	}

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state != T_SEMICOLON){
		return SYNTAX_ERR;
	}


	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state == T_KEYWORD){
		if (!strcmp(token->content, "forward\0")){

			*token = tGetToken();
			if (token->state == T_ERR){
				return LEX_ERR;
			}

			if (token->state != T_SEMICOLON){
				return SYNTAX_ERR;
			}
			if (test == NULL){
				if (BTInsert(symbolTable, tmp.key, tmp) == BT_ERR){  //pokud je deklarace syntakticky spravne, vlozim prvek do tabulky symbolu a zkontroluji zda se tam jiz nenachazi
					return SEM_ERR;
				}
			}
			else{
				return SEM_ERR;										// pokud funkce byla jednou definovana, zahlasim semantickou chybu
			}

			*token = tGetToken();
			if (token->state == T_ERR){
				return LEX_ERR;
			}

			if (token->state == T_KEYWORD){
				if (!strcmp(token->content, "function\0")){
					result = defFunction(token, symbolTable);
					if (result != SYNTAX_OK)
						return result;
				}

			}

		}
		else {
			result = funcVariables(token, symbolTable, &tmp, test);
			if (result != SYNTAX_OK)
				return result;

			result = funcStatement(token, symbolTable);
			if (result != SYNTAX_OK)
				return result;

			tmp.isDefined = true;			//funkce byla definovana

			if (test == NULL){
				if (BTInsert(symbolTable, tmp.key, tmp) == BT_ERR){  //pokud je deklarace a definice syntakticky spravne, vlozim prvek do tabulky symbolu a zkontroluji zda se tam jiz nenachazi
					return SEM_ERR;
				}
			}
			else{
				test->content.isDefined = true;						// pokud funkce byla deklarovana, v promenne test mam ulozen odkaz na funkci ktera byla definovana 
			}

			if (token->state == T_KEYWORD){
				if (!strcmp(token->content, "function\0")){
					result = defFunction(token, symbolTable);
					if (result != SYNTAX_OK)
						return result;
				}
			}
		}
	}
	else {
		return SYNTAX_ERR;
	}
	
	return result;
}

// pravidlo <var_type> -> integer
//			<var_type> -> real
//			<var_type> -> boolean
//			<var_type> -> string
int varType(tToken *token, symbolTablePtr *symbolTable, symbol *s){
	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state == T_KEYWORD){
		if (!strcmp(token->content, "integer\0")){
			s->type = tInt;
			return SYNTAX_OK;
		}
		if (!strcmp(token->content, "real\0")){
			s->type = tReal;
			return SYNTAX_OK;
		}
		if (!strcmp(token->content, "boolean\0")){
			s->type = tBool;
			return SYNTAX_OK;
		}
		if (!strcmp(token->content, "string\0")){
			s->type = tString;
			return SYNTAX_OK;
		}
	}
	return SYNTAX_ERR;
	
}

int globalListNext(tToken *token, symbolTablePtr *symbolTable){
	if (token->state != T_IDENTIFICATOR){
		return result;
	}

	symbol tmp;
	tmp.key = token->content;
	tmp.isFunction = false;

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}
	if (token->state != T_COLON){
		return SYNTAX_ERR;
	}

	if ((result = varType(token, symbolTable, &tmp)) != SYNTAX_OK){
		return result;
	}

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state != T_SEMICOLON){
		return SYNTAX_ERR;
	}
	if (BTInsert(symbolTable, tmp.key, tmp) == BT_ERR){  //pokud je deklarace syntakticky spravne, vlozim prvek do tabulky symbolu a zkontroluji zda se tam jiz nenachazi
		return SEM_ERR;
	}

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state == T_IDENTIFICATOR){	// pokud nasleduje dalsi identifikator, volam funkci znova
		result = globalListNext(token, symbolTable);
		if (result != SYNTAX_OK)
			return result;
	}

	return result;
}

// pravidlo <global_list> -> id : <var_type> ; <global_list_next>
int globalList(tToken *token, symbolTablePtr *symbolTable){
	if (token->state != T_IDENTIFICATOR){
		return SYNTAX_ERR;
	}

	symbol tmp;					
	tmp.key = token->content;
	tmp.isFunction = false;

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}
	if (token->state != T_COLON){
		return SYNTAX_ERR;
	}

	if ((result = varType(token, symbolTable, &tmp)) != SYNTAX_OK){
		return result;
	}

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}
	
	if (token->state != T_SEMICOLON){
		return SYNTAX_ERR;
	}
	if (BTInsert(symbolTable, tmp.key, tmp) == BT_ERR){  //pokud je deklarace syntakticky spravne, vlozim prvek do tabulky symbolu a zkontroluji zda se tam jiz nenachazi
		return SEM_ERR;
	}

	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state == T_IDENTIFICATOR){	// pokud nasleduje dalsi identifikator, volam funkci znova
		result = globalListNext(token, symbolTable);
		if (result != SYNTAX_OK)
			return result;
	}

	return result;
}

//pravidlo <def_global> -> var <global_list>
//		   <def_global> -> eps
int defGlobal(tToken *token, symbolTablePtr *symbolTable){
	*token = tGetToken();
	if (token->state == T_ERR){
		return LEX_ERR;
	}

	if (token->state == T_KEYWORD){
		if (!strcmp(token->content, "var\0")){
			*token = tGetToken();
			if (token->state == T_ERR){
				return LEX_ERR;
			}
			result = globalList(token, symbolTable);
			if (result != SYNTAX_OK)
				return result;
		}
	}

	return result;
}

//pravidlo <program> -> <def_global> <def_function> <stat_list> <EOF>
int program(tToken *token, symbolTablePtr *symbolTable){
	result = defGlobal(token, symbolTable);
	if (result != SYNTAX_OK)
		return result;

	result = defFunction(token, symbolTable);
	if (result != SYNTAX_OK)
		return result;

	result = checkFunctions(symbolTable);			// nez spustim analyzu tela programu, zjistim zda vsechny funkce byly definovany
	if (result != SYNTAX_OK)
		return result;

	result = body(token, symbolTable);
	if (result != SYNTAX_OK)
		return result;

	return result;
}


int parse(symbolTablePtr *symbolTable){
	result = SYNTAX_OK;
	tToken token;
	result = program(&token, symbolTable);

	return result;
}
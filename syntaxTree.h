#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

	// 标识符的长度上限
	#define IDSIZE 32
	
	// 定义语法单元和词法单元的类型
	enum { TYPETOKEN=0, TYPESYMBOL };
	
	// 定义TYPE的两种类型及名称
	enum { TYPEINT=0, TYPEFLOAT};

	// 定义语法树节点的类型
	enum { type_INT=0, type_FLOAT,type_ID,
	type_SEMI,type_COMMA,type_ASSIGNOP,type_OR,
	type_AND,type_RELOP,type_PLUS,type_MINUS,type_STAR,
	type_DIV,type_NOT,type_DOT,type_LP,type_RP,type_LB,
	type_RB,type_TYPE,type_LC,type_RC,type_STRUCT,type_RETURN,
	type_IF,type_ELSE,type_WHILE,type_Program,type_ExtDefList,
	type_ExtDef,type_ExtDecList,type_Specifier,type_StructSpecifier,
	type_OptTag,type_Tag,type_VarDec,type_FunDec,type_VarList,
	type_ParamDec,type_CompSt,type_StmtList,type_Stmt,type_DefList,
	type_Def,type_DecList,type_Dec,type_Exp,type_Args};

	// 语法树字节点的个数上限
	#define NODESIZE 9
	// 节点名字的长度上限
	#define NAMESIZE 32

	/********** define the tree struct here **********/
	//语法树节点的结构
	typedef struct TREENODE{
		int type_node;
		int type_syn;
		char name[NAMESIZE];
		int typeTableIndex;
		int symbolTableIndex;
		union{
			struct SYMBOL{
				int line;
				int column;
			}symbol;
			struct {
				union{
					int type_int;
					float type_float;
					char type_id[IDSIZE];
					char type_relop[5];
				};
				int line;
				int column;
			}token; 
		};
		//定义子树的个数，和指向子树的指针
		int num_nexts;
		struct TREENODE* next[NODESIZE];	
	}TreeNode;


#endif

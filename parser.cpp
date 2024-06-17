#include <iostream>
#include <sstream>
#include <string>

enum token_type
{
	lobj = -1, // {
	robj = -2, // }
	larr = -3, // [
	rarr = -4, // ]
	colon = -5,
	string = -6,
	number = -7,
	true_tok = -8,
	false_tok = -9,
	null_tok = -10,
	eof_tok = -11,
	error = -12,
	comma_tok = -13
};

struct token
{
	token_type token;
	std::string lexeme;
};

typedef struct token Token;

class Lexer
{
private:
	std::string source;
	int cur_idx = 0;

public:
	Lexer()
	{
		// TODO: extract to other class to generalize input
		std::stringstream buf;
		buf << std::cin.rdbuf();
		source = buf.str();
	}
	// TODO: Ugly function, please refactor duplicated codes.
	Token getToken()
	{
		Token result;
		// skip white spaces
		while ((cur_idx < source.length()) && (source[cur_idx] == '\n' || source[cur_idx] == '\t' || source[cur_idx] == ' '))
		{
			cur_idx++;
		}
		if (cur_idx == source.length())
		{
			result.token = eof_tok;
			return result;
		}
		switch (source[cur_idx])
		{
		case '{':
			result.token = lobj;
			result.lexeme += source[cur_idx];
			break;
		case '}':
			result.token = robj;
			result.lexeme += source[cur_idx];
			break;
		case '[':
			result.token = larr;
			result.lexeme += source[cur_idx];
			break;
		case ']':
			result.token = rarr;
			result.lexeme += source[cur_idx];
			break;
		case ':':
			result.token = colon;
			result.lexeme += source[cur_idx];
			break;
		case '\"':
			// consume the string until you encounter a matching ", but not followed by
			// error happens when curidex+1 goes out of bound, without matching "
			cur_idx++;
			while (true)
			{
				if (cur_idx == source.length())
				{
					result.token = error;
					return result;
				}
				if ((source[cur_idx] != '\"') || (source[cur_idx] == '\"' && source[cur_idx - 1] == '\\'))
				{
					result.lexeme += source[cur_idx];
					cur_idx++;
				}
				else
				{
					break;
				}
			}
			result.token = string;
			break;
		case ',':
			result.token = comma_tok;
			result.lexeme += source[cur_idx];
			break;
		case 't':
			cur_idx++;
			if (source.length() - cur_idx < 3)
			{
				result.token = error;
				return result;
			}
			for (int i = 0; i < 3; i++)
			{
				std::string match = "rue";
				if (source[cur_idx + i] != match[i])
				{
					result.token = error;
					return result;
				}
			}
			cur_idx += 2;
			result.token = true_tok;
			result.lexeme = "true";
			break;
		case 'f':
			cur_idx++;
			if (source.length() - cur_idx < 4)
			{
				result.token = error;
				return result;
			}
			for (int i = 0; i < 4; i++)
			{
				std::string match = "alse";
				if (source[cur_idx + i] != match[i])
				{
					result.token = error;
					return result;
				}
			}
			cur_idx += 3;
			result.token = false_tok;
			result.lexeme = "false";
			break;
		case 'n':
			cur_idx++;
			if (source.length() - cur_idx < 3)
			{
				result.token = error;
				return result;
			}
			for (int i = 0; i < 3; i++)
			{
				std::string match = "ull";
				if (source[cur_idx + i] != match[i])
				{
					result.token = error;
					return result;
				}
			}
			cur_idx += 2;
			result.token = null_tok;
			result.lexeme = "null";
			break;
		default:
			// if not a number then return error
			if (source[cur_idx] >= '0' && source[cur_idx] <= '9')
			{
				result.lexeme += source[cur_idx];
				while ((cur_idx < source.length()) && (source[cur_idx + 1] >= '0' && source[cur_idx + 1] <= '9'))
				{
					result.lexeme += source[++cur_idx];
				}
				result.token = number;
			}
			else
			{
				result.token = error;
				return result;
			}
			break;
		}
		cur_idx++;
		return result;
	}
};

struct NodeAst
{
	virtual void print(int level) = 0;
	std::vector<NodeAst *> children;
	virtual ~NodeAst() = default;
};

struct ValAst : NodeAst
{
	// TODO: add other types
	// based on type one of them will be populated
	// I know this is a very bad design, not so good with cpp yet.
	std::string stringval;
	NodeAst *obj;
	int numval;
	int type;
	void print(int level)
	{
		int t = level;
		while (level-- > 0)
			std::cout << "  ";
		if (type == string)
			std::cout << "ValAst(String) | " << stringval << '\n';
		else if (type == number)
			std::cout << "ValAst(Number) | " << numval << '\n';
		else if (type == lobj)
		{
			std::cout << "ValAst(Object) |\n";
			obj->print(t + 1);
		}
		else if (type == larr)
		{
			std::cout << "ValAst(Arr) |\n";
			obj->print(t + 1);
		}
	}
};
struct ArrayAst : NodeAst
{
	std::vector<ValAst *> elems;
	void print(int level)
	{
		int t = level;
		while (level-- > 0)
			std::cout << "  ";
		std::cout << "ArrayAst |" << '\n';
		for (auto e : elems)
			e->print(t + 1);
	}
};
struct KeyValAst : NodeAst
{
	std::string key;
	ValAst *value;
	bool isEpsilon = false;
	void print(int level)
	{
		int t = level;
		while (level-- > 0)
			std::cout << "  ";
		std::cout << "KeyValAst | " << key << '\n';
		value->print(t + 1);
	}
};
struct ObjAst : NodeAst
{
	std::vector<KeyValAst *> keyVals;
	void print(int level)
	{
		int t = level;
		while (level-- > 0)
			std::cout << "  ";
		std::cout << "ObjAst | " << '\n';
		for (auto e : keyVals)
			e->print(t + 1);
	}
};

struct KeyValAst *parseKeyVal(Lexer &lexer);

struct ObjAst *parseObjAfterL(Lexer &lexer, Token &t)
{

	ObjAst *res = new ObjAst();
	do
	{
		KeyValAst *keyval = parseKeyVal(lexer);
		if (keyval == nullptr)
			return nullptr;
		if (keyval->children.size() != 0)
		{
			res->keyVals.push_back(keyval);
			res->children.push_back(keyval);
			t = lexer.getToken();
			if (t.token == robj)
				return res;
		}
		else
		{
			// currently I ignore the comma
			// TODO: Decide if multiple commas followed by } is a well formed json or not
			return res;
		}
	} while (t.token == comma_tok);
	return nullptr;
}

struct ValAst *parseValAfter(Lexer &lexer, Token &t);
struct ArrayAst *parseArray(Lexer &lexer)
{
	ArrayAst *res = new ArrayAst();
	Token t;
	do
	{
		t = lexer.getToken();
		if (t.token == rarr)
			return res;
		ValAst *v = parseValAfter(lexer, t);
		if (v == nullptr)
		{
			return nullptr;
		}
		res->elems.push_back(v);
		t = lexer.getToken();
		if (t.token != comma_tok && t.token != rarr)
		{
			return nullptr;
		}
	} while (t.token == comma_tok);
	if (t.token != rarr)
	{
		return nullptr;
	}
	return res;
}

struct ValAst *parseValAfter(Lexer &lexer, Token &t)
{
	if (t.token == string)
	{
		ValAst *val = new ValAst();
		val->stringval = t.lexeme;
		val->type = t.token;
		return val;
	}
	else if (t.token == number)
	{
		ValAst *val = new ValAst();
		val->numval = stoi(t.lexeme);
		val->type = t.token;
		return val;
	}
	else if (t.token == true_tok || t.token == false_tok || t.token == null_tok)
	{
		ValAst *val = new ValAst();
		val->stringval = t.lexeme;
		val->type = t.token;
		return val;
	}
	else if (t.token == larr)
	{
		ValAst *val = new ValAst();
		val->obj = parseArray(lexer);
		if (val->obj == nullptr)
			return nullptr;
		val->type = larr;
		return val;
	}
	else if (t.token == lobj)
	{
		ValAst *val = new ValAst();
		val->obj = parseObjAfterL(lexer, t);
		if (val->obj == nullptr)
			return nullptr;
		val->type = lobj;
		return val;
	}
	return nullptr;
}

struct ValAst *parseVal(Lexer &lexer)
{
	Token t = lexer.getToken();
	return parseValAfter(lexer, t);
}

struct KeyValAst *parseKeyVal(Lexer &lexer)
{
	struct KeyValAst *result = new KeyValAst();
	Token t = lexer.getToken();
	// handle the epsilon production
	if (t.token == robj)
	{
		result->isEpsilon = true;
		return result;
	}
	if (t.token != string)
		return nullptr;
	result->key = t.lexeme;

	t = lexer.getToken();
	if (t.token != colon)
		return nullptr;

	ValAst *val = parseVal(lexer);
	if (val == nullptr)
		return nullptr;
	result->value = val;
	result->children.push_back(val);

	return result;
}

struct ObjAst *parseObject(Lexer &lexer)
{
	Token t = lexer.getToken();
	if (t.token != lobj)
		return nullptr;

	return parseObjAfterL(lexer, t);
}

int main()
{
	Lexer lexer;
	ObjAst *res = parseObject(lexer);
	if (res != nullptr)
	{
		res->print(0);
		printf("Parsed successfuly!\n");
	}
	else
		printf("Error in parsing\n");
	return 0;
}

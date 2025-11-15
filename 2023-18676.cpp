#include<iostream>
#include<string>
using namespace std;

//====================================

int globalRoot; //root is global (need to access in mem class)

//====================================
bool isStringEmpty(string& s) {
    return s == "";
}

void makeLower(string& str) {
    string ret = str;
    for (int i=0; i<str.size(); i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') ret[i] = str[i] + 32;
    }
    str = ret;
}

bool isDouble(const string &str) {
    try{
        stod(str);
    }
    catch (const std::invalid_argument&) {
        return false;
    }
    return true;
}

void error() {
    cout << "error" <<endl;
}

class HashUpdateStruct {
public: 
    int newHashValue = 0;
};

class Node {
public: 
    int lchild, rchild;
};

class Memory{
public:
    Node* array;
    int capacity;
    int freeListRoot;

    Memory(int capacity) {
        this->capacity = capacity;
        array = new Node[capacity];
        array[0].lchild = 0;
        array[0].rchild = 0;
        for (int i=1; i<capacity; i++) {
            array[i].lchild = 0; //no element contain
            array[i].rchild = i+1; //likned to next node
        }
        array[capacity-1].rchild = 0; //last node points null
        freeListRoot = 1;
    }
    ~Memory() {
        delete[] array;
    }
    int alloc() {
        if (array[freeListRoot].rchild == 0) doubling(); //freeListRoot is only freeSpace exists > doubling
        int ret = freeListRoot;
        freeListRoot = array[freeListRoot].rchild;
        return ret;
    }
    void deAlloc(int idx) {
        if (idx <= 0) return; //input is hash value or null
        else {
            deAlloc(array[idx].lchild);
            deAlloc(array[idx].rchild);
        }
        int oldFreeListRoot = freeListRoot;
        freeListRoot = idx;
        array[freeListRoot].lchild = 0; //not necessary but for safeness
        array[freeListRoot].rchild = oldFreeListRoot;
    }
    void doubling() {
        Node* newArray = new Node[2*capacity];
        for (int i=0; i<capacity; i++) {
            newArray[i] = array[i];
        }
        for (int j = capacity-1; j<2*capacity; j++) {
            newArray[j].lchild = 0; //no element
            newArray[j].rchild = j+1; //linked to next node
        }
        newArray[2*capacity-1].rchild = 0; //last node points null
        newArray[freeListRoot].rchild = capacity; //link space to current freeListRoot
        capacity *= 2;
        delete[] array;
        array = newArray;
    }
    void setLchild(int idx, int value) {
        array[idx].lchild = value;
    }
    void setRchild(int idx, int value) {
        array[idx].rchild = value;
    }
    int getLchild(int idx) {
        return array[idx].lchild;
    }
    int getRchild(int idx) {
        return array[idx].rchild;
    }
    int getFreeListRoot() {
        return freeListRoot;
    }
    void printNodeArray() {
        cout << "==NODE ARRAY==" << endl;
        cout << "idx lchild rchild" << endl;
        for (int i=1; i<freeListRoot; i++) {
            cout << i << " "<<  array[i].lchild <<" " << array[i].rchild << endl;
        }
    }
    void updateParseTree(HashUpdateStruct* hashUpdateStruct, int idx) {
        if (getLchild(idx) < 0) {
            setLchild(idx, hashUpdateStruct[-getLchild(idx)].newHashValue);
        } 
        else if (getLchild(idx) == 0) {}
        else {// L child is node
            updateParseTree(hashUpdateStruct, getLchild(idx));
        }
        if (getRchild(idx) == 0) {}
        else {
            updateParseTree(hashUpdateStruct, getRchild(idx));
        }
    }
};

class Hash{
public:
    string symbol = "";
    int linkValue = 111; //out of table range mean not link just string

    bool isEmpty() {
        return symbol == "";
    }
};

class HashTable{
public: 
    Hash* table;
    Memory* memory;
    int capacity;
    
    HashTable(int capacity, Memory* memory) {
        this->capacity = capacity;
        this->memory = memory;
        table = new Hash[capacity];
    }
    ~HashTable() {
        delete[] table;
    }
    int getHashValue(string input) {
        int idx = findHashIdx(input);
        if (idx != -1) return -idx; //hash already exixsts in table

        idx = insertNewHashRetIdx(input); // insertNewHash
        if (idx == -1) { //hash table overflow
            makeHashTableDouble();
            idx = insertNewHashRetIdx(input);
        }
        return -idx;
    }
    void makeHashTableDouble() {
        // cout << "before doubling:" << endl;
        // memory->printNodeArray();
        // printTable();
        string* currentSymbols = new string[capacity];
        int currentSymIdx =0;
        for (int i=0; i<capacity; i++) {
            if (!table[i].isEmpty()) {
                currentSymbols[currentSymIdx++] = table[i].symbol;
            }
        }
        Hash* oldTable = table;
        int oldCapacity = capacity;
        capacity *= 2;
        table = new Hash[capacity];

        HashUpdateStruct* hashUpdateStruct = new HashUpdateStruct[oldCapacity];
        makeUpdateHashTableAndInsertNewHashs(oldTable, oldCapacity, hashUpdateStruct);

        if (globalRoot < 0) {
            int oldIdx = -globalRoot;
            globalRoot = hashUpdateStruct[oldIdx].newHashValue;
        } else if (globalRoot > 0) {
            memory->updateParseTree(hashUpdateStruct, globalRoot);
        }

        delete[] oldTable;
        delete[] hashUpdateStruct;

        // // cout << "After doubling:" << endl;
        // memory->printNodeArray();
        // printTable();

    }
    void makeUpdateHashTableAndInsertNewHashs(Hash* oldTable, int oldCapacity, HashUpdateStruct* hashUpdateStruct) {
        for (int i=1; i<oldCapacity; i++) {
            hashUpdateStruct[i].newHashValue = -insertNewHashRetIdx(oldTable[i].symbol);
        }
    }
    int findHashIdx(string& input) {
        int tempIdx;
        for (int i=0; i<capacity; i++) {
            tempIdx = hashFunc(input, i);
            if (table[tempIdx].symbol == input) 
            return tempIdx;
        }
        return -1;
    }
    string findHashSymbol(int idx) {
        return table[idx].symbol;
    }
    double getVal(int hashValue) {
        int idx = -hashValue;

        if (table[idx].linkValue < 0) {
            return getVal(table[idx].linkValue);
        }
        else if (table[idx].linkValue == 111) {
            string symbol = table[idx].symbol; //double 아니면 죽음
            return stod(symbol);
        }
        else {
            error();
            return 0;
        }
    }
    
    int insertNewHashRetIdx(string input) {
        if (input == "") return 0;
        int tempIdx;
        for (int i=0; i<capacity; i++) {
            tempIdx = hashFunc(input, i);
            if (table[tempIdx].isEmpty() && tempIdx != 0) {
                table[tempIdx].symbol = input;
                return tempIdx;
            }
        }
        return -1; //overflow
    }
    int hashFunc(string& input, int i) {
        int sum =0;
        for (int j=0; j<input.size(); j++) sum += input[j];
        return (sum + i) % capacity;
    }
    int getCapacity() {
        return capacity;
    }
    string getHashSymbolByIdx(int idx) {
        return table[idx].symbol;
    }

    void setLinkValue(int idx, int linkValue) {
        table[-idx].linkValue = linkValue;
    }
    int getLinkValue(int idx) {
        return table[-idx].linkValue;
    }
    void printTable() {
        cout << "==HASH TABLE==" << endl;
        cout << "hashValue symbol"<< endl;
        for (int i=1; i<capacity; i++) {
            if (!table[i].isEmpty()) {
            cout << -i<< " " << table[i].symbol << ' ' << table[i].linkValue <<endl;
            }
        }
    }
    void hashInitInsert(string* tokens, int tokenSize) {
        for(int i=0; i<tokenSize; i++) {
            getHashValue(tokens[i]);
        }
    }
};

class Tokens{
public: 
    string* tokens;
    int size;
    int currentTokenIdx = 0;

    Tokens(string& input) {
        if (input.size() == 0) {
            size = 0;
            return; 
        }
        makeLower(input);
        string* tempTokens = new string[input.size()];
        string temp = "";
        int tokenIdx = 0;
        for (int i=0; i<input.length(); i++) {
            if (input[i] == '(' or input[i] == ')') {
                if (!isStringEmpty(temp)) tempTokens[tokenIdx++] = temp;
                tempTokens[tokenIdx++] = input[i];
                temp = "";
            }
            else if (input[i] == ' ') {
                if (!isStringEmpty(temp)) tempTokens[tokenIdx++] = temp;
                temp = "";
            }
            else if (input[i] == '\'') {
                if (!isStringEmpty(temp)) tempTokens[tokenIdx++] = temp;
                tempTokens[tokenIdx++] = input[i];
                temp = "";
            }
            else {
                temp += input[i];
            }
        }
        if (temp == "" && tokenIdx == 0) {//not meaningful input
            size = 0;
            return;
        }
        if (temp != "") tempTokens[tokenIdx++] = temp;
        size = tokenIdx;
        tokens = new string[size];
        for (int i=0; i<size; i++) tokens[i] = tempTokens[i];
        delete[] tempTokens;
    }
    ~Tokens() {
        delete[] tokens;
    }
    string getNextToken() {
        return tokens[currentTokenIdx++];
    }
    void pushBack() {
        currentTokenIdx--;
    }
    bool isTokenEnd() {
        return size == currentTokenIdx;
    }
    void setTokenIdxZero() {
        currentTokenIdx  = 0;
    }
    // void print() {
    //     for (int i=0; i<size; i++) {
    //         cout << tokens[i] << endl;
    //     }
    // }
};

class StackElement {
public:
    int hashValue;
    int linkValue;
};

class Stack {
public:
    int capacity;
    int top;
    StackElement *stack;

    Stack(int capacity) {
        stack = new StackElement[capacity];
        top = 0;
        for(int i = 0; i < capacity; ++i) {
            stack[i].hashValue = 0;
            stack[i].linkValue = 0;
        }
    }
    ~Stack(void) {
        delete[] stack;
    }
    void push(StackElement x) {
        stack[top++] = x;
    }
    StackElement pop(void) {
        if(top == 0) return StackElement{0, 0};
        return stack[--top];
    }
    bool isEmpty(void) {
        return top == 0;
    }
};

int read(Memory& memory, HashTable& hashTable, Tokens& tokens) {
    int root = 0;
    int temp;
    bool first = true;
    int tokenHashValue = hashTable.getHashValue(tokens.getNextToken());
    if (tokenHashValue == hashTable.getHashValue("(")) {
        tokenHashValue = hashTable.getHashValue(tokens.getNextToken());
        while (tokenHashValue != hashTable.getHashValue(")")) {
            if (first) {
                temp = memory.alloc();
                root = temp;
                first = false;
            }
            else {
                memory.setRchild(temp, memory.alloc());
                temp = memory.getRchild(temp);
            }
            if (tokenHashValue == hashTable.getHashValue("(")) {
                tokens.pushBack();
                memory.setLchild(temp, read(memory, hashTable, tokens));
            }
            else {
                // cout << "setLchild, temp: " << temp << " tokenHasht: " << tokenHashValue << endl;
                memory.setLchild(temp, tokenHashValue);
            }
            memory.setRchild(temp, 0);
            // memory.printNodeArray();
            tokenHashValue = hashTable.getHashValue(tokens.getNextToken());
        }
        return root;
    }
    else return tokenHashValue;
}

void print(int idx, bool isStartList, Memory& memory, HashTable& hashTable) {
    if (idx == 0 && isStartList) {
        cout << "()";
    }
    else if (idx < 0) {
        cout << hashTable.getHashSymbolByIdx(-idx) << " ";
    }
    else if (idx > 0) {
        if (isStartList) {
            cout << "( ";
        }
        print(memory.getLchild(idx), true, memory, hashTable);
        if (memory.getRchild(idx) != 0) {
            print(memory.getRchild(idx), false, memory, hashTable);
        }
        else cout << ") ";
    }
}

string preprocessing(string newCommand, Tokens& tokens) {
    string token;
    while (!tokens.isTokenEnd()) {
        token = tokens.getNextToken();
        if (token == "define") {
            newCommand = newCommand +  " define ";
            token = tokens.getNextToken();
            if (token == "(") {
                token = tokens.getNextToken();
                newCommand = newCommand + " " + token + " (lambda( " +  preprocessing("", tokens) + " ) ";
            } 
            else {
                newCommand = newCommand + " " + token + " ";
            }
        }
        else if (token == "\'") {
            newCommand = newCommand + " (quote ";
            int numOfLeftParen = 0;
            do {
                token = tokens.getNextToken();
                newCommand = newCommand + " " + token + " ";
                if (token == "(") {
                    numOfLeftParen += 1;
                }
                else if (token == ")") {
                    numOfLeftParen -= 1;
                }
            } while (numOfLeftParen > 0);
            newCommand = newCommand + " ) ";
        }
        else {
            newCommand = newCommand + " " +  token + " ";
        }
    }
    return newCommand;
}

int eval(Memory& memory, HashTable& hashTable, int root) {//노드 혹은 hashvalue를 반환
    if (root < 0) {
        if(hashTable.getLinkValue(root) == 0) {
            return 0;
        }
        else if (hashTable.getLinkValue(root) == 111) { 
            return root;
        }
        else {
            return eval(memory, hashTable, hashTable.getLinkValue(root));
        }
    }
    if(root==0) return 0;
    
    int operatorHash = memory.getLchild(root); //root 는 리스트의 시작이니까 lchlid는 항상 operator이다

    if (operatorHash == hashTable.getHashValue("+")) {
        return hashTable.getHashValue(to_string(hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(root)))) + 
            hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(root)))))));
    } 
    else if (operatorHash == hashTable.getHashValue("-")) {
        return hashTable.getHashValue(to_string(hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(root)))) -   
            hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(root)))))));
    }
    else if (operatorHash == hashTable.getHashValue("*")) {  
        return hashTable.getHashValue(to_string(hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(root)))) * 
            hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(root)))))));
    }
    else if (operatorHash == hashTable.getHashValue("/")) {
        return hashTable.getHashValue(to_string(hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(root)))) / 
            hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(root)))))));
    }
    else if (operatorHash == hashTable.getHashValue(">")) {
        int arg1 = hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(root))));
        int arg2 = hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(root)))));
        if (arg1 > arg2) return hashTable.getHashValue("#t");
        else return hashTable.getHashValue("#f");
    }
    else if (operatorHash == hashTable.getHashValue("<")) {
        int arg1 = hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(root))));
        int arg2 = hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(root)))));
        if (arg1 < arg2) return hashTable.getHashValue("#t");
        else return hashTable.getHashValue("#f");
    }
    else if (operatorHash == hashTable.getHashValue("=")) {
        int arg1 = hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(root))));
        int arg2 = hashTable.getVal(eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(root)))));
        if (arg1 == arg2) return hashTable.getHashValue("#t");
        else return hashTable.getHashValue("#f");
    }
    else if (operatorHash == hashTable.getHashValue("number?")) {
        int evalReturn = eval(memory, hashTable, memory.getLchild(memory.getRchild(root)));
        if (evalReturn < 0) {
            if(isDouble(hashTable.getHashSymbolByIdx(-evalReturn)))
                return hashTable.getHashValue("#t");
        }
        return hashTable.getHashValue("#f");
    }
    else if (operatorHash == hashTable.getHashValue("symbol?")) {
        int evalReturn = eval(memory, hashTable, memory.getLchild(memory.getRchild(root)));
        if(evalReturn == 0) return hashTable.getHashValue("#f");
        if (evalReturn < 0) {
            if(isDouble(hashTable.getHashSymbolByIdx(-evalReturn)))
                return hashTable.getHashValue("#f");
        }
        return hashTable.getHashValue("#t");
    }
    else if (operatorHash == hashTable.getHashValue("null?")) {
        int arg = memory.getLchild(memory.getRchild(root));

        // 인자가 음수
        if (arg < 0 ) {
            if (hashTable.getLinkValue(arg) == 0) return hashTable.getHashValue("#t");
            else if (hashTable.getLinkValue(arg) == 111) {
                //undefined 를 null? 인지 체크함
                error();
            }
        }
        else if (arg == 0) return hashTable.getHashValue("#t");
        else {//arg가 양수
            int result = eval(memory, hashTable, arg);
            if (result == 0) return hashTable.getHashValue("#t");
            else return hashTable.getHashValue("#f");
        }

    }
    else if (operatorHash == hashTable.getHashValue("cons")) {
        int newMemory = memory.alloc();
        memory.setLchild(newMemory, eval(memory, hashTable, memory.getLchild(memory.getRchild(root))));
        memory.setRchild(newMemory, eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(root)))));
        return newMemory;
    }
    else if (operatorHash == hashTable.getHashValue("if")) {
        int condition = eval(memory, hashTable, memory.getLchild(memory.getRchild(root)));
        if (condition == hashTable.getHashValue("#t")) {
            return eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(root))));
        }
        else {  
            return eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(memory.getRchild(root)))));
        }
    }
    else if (operatorHash == hashTable.getHashValue("cond")) {
        while (memory.getRchild(memory.getRchild(root)) != 0) {
            root = memory.getRchild(root);
            if (eval(memory, hashTable, memory.getLchild(memory.getLchild(root))) == hashTable.getHashValue("#t")) {
                return eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getLchild(root))));
            }
        }
        if (memory.getLchild(memory.getLchild(memory.getRchild(root))) !=  hashTable.getHashValue("else")) {
            error();
        }
        return eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getLchild(memory.getRchild(root)))));
    }
    else if (operatorHash == hashTable.getHashValue("car")) {
        return memory.getLchild(eval(memory, hashTable, memory.getLchild(memory.getRchild(root))));
    }
    else if (operatorHash == hashTable.getHashValue("cdr")) {
        return memory.getRchild(eval(memory, hashTable, memory.getLchild(memory.getRchild(root))));
    }
    else if (operatorHash == hashTable.getHashValue("define")) {
        if (memory.getLchild(memory.getLchild(memory.getRchild(memory.getRchild(root)))) == hashTable.getHashValue("lambda")) {
            hashTable.setLinkValue(memory.getLchild(memory.getRchild(root)), 
                memory.getLchild(memory.getRchild(memory.getRchild(root))));
        }
        else {
            hashTable.setLinkValue(memory.getLchild(memory.getRchild(root)), 
                eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(root)))));
        }
    }
    else if (operatorHash == hashTable.getHashValue("quote")) {
        return memory.getLchild(memory.getRchild(root));
    }
    else if (operatorHash == hashTable.getHashValue("lambda")) {
        return eval(memory, hashTable, memory.getLchild(memory.getRchild(memory.getRchild(root))));
    }
    else if (operatorHash < 0 && hashTable.getLinkValue(operatorHash) > 0 && hashTable.getLinkValue(operatorHash) != 111) {      // user defined function
        int parameterRoot = memory.getLchild(memory.getRchild(hashTable.getLinkValue(operatorHash)));
        
        Stack stack(101);
        if (parameterRoot != 0) {     // 인자가 존재함
            int parameterTemp = parameterRoot;
            int inputTemp = memory.getRchild(root);
            while (parameterTemp != 0) {
                StackElement tempStackElement = {memory.getLchild(parameterTemp), hashTable.getLinkValue(memory.getLchild(parameterTemp))};
                stack.push(tempStackElement); //current save
                hashTable.setLinkValue(memory.getLchild(parameterTemp), eval(memory, hashTable, memory.getLchild(inputTemp))); //파라미터를 input으로 link
                parameterTemp = memory.getRchild(parameterTemp);
                inputTemp = memory.getRchild(inputTemp);
            }    
        }
        int returnValue = eval(memory, hashTable, hashTable.getLinkValue(operatorHash));
        while(!stack.isEmpty()) {
            StackElement tempStackElement = stack.pop();
            hashTable.setLinkValue(tempStackElement.hashValue, tempStackElement.linkValue);
        }
        return returnValue;
    }
    
    return root;

}

Memory memory(101); 
HashTable hashTable(101, &memory); 

int main() {
    // //inner operator
    // string* inner = {"+", "-", "*", "/", "#t", "#f", "null?", "number?", "symbol?"};
    // hashTable.hashInitInsert(inner, inner.size());

    while (true) {
        string input;
        getline(cin, input);
        if (input == "no") {break;}
        Tokens Ctokens(input);
        // Ctokens.print();
        if (Ctokens.size == 0) {continue;} //no meaningful input
        string newCommand = "";
        string newInput = preprocessing(newCommand, Ctokens); //make new command to string
        Tokens newTokens(newInput); //make new string to new tokens
        hashTable.hashInitInsert(newTokens.tokens, newTokens.size); //in read, token hashValue cannot changed while doubling so do it prevois
        globalRoot = read(memory, hashTable, newTokens);
        int result = eval(memory, hashTable, globalRoot); //hashValue이거나 node index임


        cout << "Free list's root = " << memory.getFreeListRoot() << endl; //single elemnet's root is hashValue
        //if negative integer, global root is hashValue and actual root is 0
        if (globalRoot < 0) cout << "Parse tree's root = " << 0 << endl; 
        else cout << "Parse tree's root = " << globalRoot << endl;

        memory.printNodeArray();
        hashTable.printTable();
        cout << "PRINT: " << endl;
        
        print(result, true, memory, hashTable);
        cout << endl;
        
        // memory.deAlloc(globalRoot);
    }

    return 0;
}
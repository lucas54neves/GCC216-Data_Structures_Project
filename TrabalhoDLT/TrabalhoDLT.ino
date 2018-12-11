
/* 
  Trabalho de Estruturas de Dados (GCC216)
  Cadastro de temperatura e luminosidade em um sistema embarcado
  Copyright 2018 by Davi Horner, Lucas Neves, Thiago Luigi 
  Arquivo lê dados do ambiente (temperatura e luminosidade) e armazena inicialmente em uma fila e posteriormente em uma tabela hash
*/

#include <SPI.h>
#include <SD.h>

const int LM35 = 0;
float celsius = 0;
int ADClido = 0;
const int LDR = 1;
int lumens = 0;
const int BotaoI = 2;
const int BotaoF = 3;
int EstadoInserir = 0;
int EstadoFinalizar = 0;
int milseg = 0;

class Noh{
  friend class Fila;
  friend class TabelaHash;
  private:
    float mTemp;
    int mTime;
    int mLuz;
    Noh* mPtProx;
  public:
    Noh(float celsius, int milseg, int lumens);
};

Noh::Noh(float celsius, int milseg, int lumens) {
  mPtProx = NULL;
  mTemp = celsius;
  mTime = milseg;
  mLuz = lumens;
}

//Começo-Fila

class Fila {
  friend class TabelaHash;
  public:
    // Constroi uma fila vazia.
    Fila();
    // Desaloca memória de todos os nós da fila.
    ~Fila();
    Noh* Remover();
    // Insere um item no final da fila.
    void Inserir(float celsius, int milseg, int lumens);
    // Remove todos os itens da fila.
    void LimparTudo();
    // Consulta se a fila está vaiza.
    bool Vazia() const;
  private:
    Noh* mPrimeiro;
    Noh* mUltimo;
    unsigned mTamanho;
};

Fila::Fila() {
  mTamanho = 0;
  mUltimo = NULL;
  mPrimeiro = NULL;
}

Fila::~Fila() {
  LimparTudo();
}

void Fila::Inserir(float celsius, int milseg, int lumens) {
  Noh *temp = new Noh(celsius, milseg, lumens);
  
  if (Vazia()) {
    mPrimeiro = temp;
    mUltimo = temp;
    mTamanho++;
  } 
  
  if (mUltimo != NULL) {
    if (mTamanho == 15) {
      Serial.println("Limite da fila quase atingido. Faltam 5 inserções.");
    }
    
    if (mTamanho <= 20) {
      mUltimo->mPtProx = temp;
      mUltimo = temp;
      mTamanho++;
    } else {
      Serial.println("Limite da fila atingido. Dado mais antigo será removido.");
      mUltimo->mPtProx = temp;
      mUltimo = temp;
      mTamanho++;
      Remover();
    }
  }
}


void Fila::LimparTudo() {
  if(mUltimo == NULL){
     Serial.print("Fila já está vazia");
  }else{
    while(mPrimeiro != NULL){
      Remover();
    }
  }
}

Noh* Fila::Remover() {
  if(Vazia()){
    Serial.print("Fila já está vazia");
  }else{
    Noh* temp = mPrimeiro;
    Noh* mensageiro = temp;
    mPrimeiro = mPrimeiro->mPtProx;
    mensageiro->mPtProx = NULL;
    mTamanho--;
    Serial.println("Removendo... Aguarde.");
    delay(1000);
    delete temp;
    delay(1000);
    Serial.println("Removido.");
    return mensageiro;
  }
}

bool Fila::Vazia() const {
  if(mPrimeiro == NULL){
    return true;
  }else{
    return false;
  }
}


//Fim-Fila

//Começo-Hash

int UMPRIMO = 13; 

unsigned int funcaoHash (int milseg){
  unsigned int h;
  // Função hash de teste
  //h = UMPRIMO++ % 24;
  // Função hash oficial
  // Cada posição na hash equivale a uma hora do dia
  h = (milseg / 3600000 ) % 24;
  return h;
}

class TabelaHash {
  private:
    Noh** mElementos;
    int mCapacidade;
  public:
    TabelaHash();
    ~TabelaHash();
    void Inserir(Noh* novo);
    void Percorre(File myFile);
};

TabelaHash::TabelaHash() {
  mCapacidade = 24;
  mElementos = new Noh*[mCapacidade];
  for (int i = 0; i<mCapacidade; i++) {
    mElementos[i] = NULL;
  }
}

TabelaHash::~TabelaHash() {
  for (int i=0; i < mCapacidade; i++) {
    Noh* atual = mElementos[i];
    while (atual != NULL) {
      Noh* aux = atual;
      atual = atual->mPtProx;
      delete aux;
    }
  }
  delete[] mElementos;
}

void TabelaHash::Inserir(Noh* novo) {
  int h;
  h = funcaoHash(novo->mTime);
  if (mElementos[h] == NULL) {
    mElementos[h] = novo;
  } else {
    Noh* local = mElementos[h];
    while (local->mPtProx != NULL) {
      local = local->mPtProx;
    }
    local->mPtProx = novo;
  }
  delay(1000);
}

void TabelaHash::Percorre(File myFile) {
  Noh* atual;
  for (int i = 0; i < mCapacidade; i++) {
    myFile.print(i);
    Serial.print(i);
    myFile.print(": ");
    Serial.print(": ");
    atual = mElementos[i];
    while (atual != NULL) {
      
      Serial.print("[Temperatura: ");
      Serial.print(atual->mTemp);
      Serial.print(" | ");
      Serial.print("Luz: ");
      Serial.print(atual->mLuz);
      Serial.print("] -> ");

      myFile.print("[Temperatura: ");
      myFile.print(atual->mTemp);
      myFile.print(" | ");
      myFile.print("Luz: ");
      myFile.print(atual->mLuz);
      myFile.print("] -> ");
      atual = atual->mPtProx;

    }
    Serial.println("NULL");
    myFile.println("NULL");
  }
  myFile.println();
}

Fila minhaFila;
TabelaHash minhaHash;
File myFile;

void setup() {
  // Abre as comunicações seriais e espera pela porta para abrir:
  Serial.begin(9600);
  while (!Serial) {
    ; // Espera pela porta serial para conectar. Necessário apenas para porta USB nativa
  }

  pinMode(BotaoI, INPUT);
  pinMode(BotaoF, INPUT);

  Serial.print("Inicializando o cartão SD...");

  if (!SD.begin(4)) {
    Serial.println("falha na inicialização!");
    while (1);
  }
  Serial.println("inicialização feita.");

  bool ativado = true;

  while (ativado) {
    EstadoInserir = digitalRead(BotaoI);
    EstadoFinalizar = digitalRead(BotaoF);

    if(EstadoInserir == HIGH){ // Botão do canto 
      // Esse botão insere uma vez na fila a cada vez que é clicado
      // A inserção é limitada a 35
      delay(500);
      ADClido = analogRead(LM35);
      lumens = analogRead(LDR); //Luz
      celsius = ADClido * 0.1075268817;
      milseg = millis();
      Serial.print("Temperatura = ");
      Serial.print(celsius);
      Serial.println(" *C");
      Serial.print("Luminosidade = ");
      Serial.println(lumens);
      delay(500);
      minhaFila.Inserir(celsius, milseg, lumens);
      Serial.println("Espere 2 segundos para clicar novamente.");
      delay(1000);
    }

    // Abre o arquivo. Note que apenas um arquivo é aberto por vez,
    // então você tem que escolher um antes de abrir o outro
    if(EstadoFinalizar == HIGH){ // Botão do meio
      // Esse botão desenfileira a fila e insere na hash
      // Isso é feito até a fila esvaziar
      myFile = SD.open("dados.txt", FILE_WRITE);
      Serial.println("Criando a tabela hash.");
      while(!minhaFila.Vazia()){
        Noh* temp = minhaFila.Remover();
        delay(1000);
        minhaHash.Inserir(temp);
        delay(1000);
      }
      Serial.println("Tabela criada.");
      if(myFile) {  // Se o arquivo abrir corretamente, escreve nele:
        Serial.println("Escrevendo a Tabela Hash no arquivo...");
        minhaHash.Percorre(myFile);
        delay(1000);
        myFile.close();  // Fecha o arquivo
        Serial.println("escrito.");
        ativado = false;
        Serial.println("Programa finalizado");
      }  
      else {
        Serial.println("Erro abrindo o arquivo"); // Se ocorreu erro na abertura do arquivo
      }
    }
  }
}
void loop() {
  // O loop não faz nada
}

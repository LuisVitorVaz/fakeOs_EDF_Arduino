#include <avr/io.h>
#include <Arduino.h>
#include <TimerOne.h>
/*
*
* Vari�veis do Kernel
*
*/
/*Define Clock de interrup��o
 * Tabela de Interrup��o :
 * 1       ClkT = 1   segundo
 * 0.1     ClkT = 100 milissegundos
 * 0.01    Clkt = 10  milissegundos
 * 0.001   Clkt = 1   milissegundo
 * 0.0001  ClkT = 100 microssegundos
 * 0.00001 ClkT = 10  microssegundos
*/
#define ClkT 2

#define INVALID_INDEX 0xFFFF
#define Slice 1000000 //1 segundo
#define MaxNumberTask 4
#define NUM_TASKS 4
#define SizeTaskStack 64 // Tamanho da pilha da tarefa
unsigned int NumberTaskAdd=-1;
volatile int TaskRunning = 0;
#define MAX_NAME_LENGTH 30
char myName[MAX_NAME_LENGTH];
int  SchedulerAlgorithm ;

enum Scheduler{
  RR,
  RM,
  EDF
};
enum Taskstates{
  INITIAL,
  READY,
  RUNNING,
  DEAD,
  BLOCKED
};
typedef struct 
{
    int queue[MaxNumberTask];
    int  tail;
    int head;
}ReadyList;
ReadyList ready_queue;

typedef struct 
{
  short count;
  int sem_queue[MaxNumberTask], tail, header;
}sem_t;

typedef struct
{
  int CallNumber;
  unsigned char *p0;
  unsigned char *p1;
  unsigned char *p2;
  unsigned char *p3;
}Parameters;
volatile Parameters kernelargs ;


typedef struct {
    int16_t Tid;
  const char *name;
  unsigned short Prio;
  unsigned int Time;
  unsigned short Join;
  unsigned short State;
  unsigned short time_computing_static;
  unsigned short period_static;
  unsigned short time_computing_dinamic;
  unsigned int computationTimerStatic;
  unsigned int computationTimerDinamic;                // Deadline da tarefa (novo campo)
  unsigned short period_dinamic;
  uint8_t Stack[SizeTaskStack]; // Vetor de pilha
  uint8_t* P; // Ponteiro de pilha
} TaskDescriptor;
TaskDescriptor Descriptors[MaxNumberTask]; // Array de descritores de tarefas

/* 
*
*Servicos do kernel
*
*/
enum sys_temCall{
  TASKCREATE,
  SEM_WAIT,
  SEM_POST,
  SEM_INIT,
  WRITELCDN,
  WRITELCDS,
  EXITTASK,
  SLEEP,
  MSLEEP,
  USLEEP,
  LIGALED,
  DESLIGALED,
  START, 
  TASKJOIN,
  SETMYNAME,
  GETMYNAME,
  NKPRINT,
  GETMYNUMBER,
  NKREAD,
};

/*
*
* Rotinas do kernel
*
*
*/
void kernel() {
    switch(kernelargs.CallNumber){
    case TASKCREATE: // OK
      // sys_taskcreate((int *)kernelargs.p0,(void(*)())kernelargs.p1);
    sys_taskcreate((int *)kernelargs.p0, 
                   (void(*)())kernelargs.p1, 
                   (unsigned short)kernelargs.p2, 
                   (unsigned short)kernelargs.p3);
      break;
    case SEM_WAIT: // OK
     // Serial.println("SEMWAIT: ") ;
      sys_semwait((sem_t *)kernelargs.p0);
      break;
    case SEM_POST: // OK
      sys_sempost((sem_t *)kernelargs.p0);
      break;
    case SEM_INIT: // OK
      //Serial.println("SEMINIT: ") ;
      sys_seminit((sem_t *)kernelargs.p0,(int)kernelargs.p1);
      break;
    case WRITELCDN: // NAO TEREMOS
      // LCDcomando((int)arg->p1);
      // LCDnum((int)arg->p0);
      break;
    case WRITELCDS: // NAO TEREMOS
      // LCDcomando((int)arg->p1);
      // LCDputs((char*)arg->p0);
      break;
    case EXITTASK: // OK
      sys_taskexit();
      break;
    case SLEEP: 
      sys_sleep((int)kernelargs.p0);
      break;
    case MSLEEP: 
      sys_msleep((int)kernelargs.p0);
      break;
    case USLEEP: 
      sys_usleep((int)kernelargs.p0);
      break;
    case LIGALED: // OK
      sys_ligaled();
      break;
    case DESLIGALED: // OK
      sys_desligaled();
      break;
    case START: 
      sys_start((int)kernelargs.p0);
      break;
    case TASKJOIN: 
     // sys_taskjoin((int)arg->p0);
      break;
    case SETMYNAME: // OK
      sys_setmyname((const char *)kernelargs.p0);
      break;
    case GETMYNAME: // OK
      sys_getmyname((const char *)kernelargs.p0);
      break;  
    case NKPRINT: // OK
       sys_nkprint((char *)kernelargs.p0,(void *)kernelargs.p1);
       break;
    case GETMYNUMBER: 
       sys_getmynumber((int *)kernelargs.p0);
       break;
    case NKREAD: 
       //sys_nkread((char *)arg->p0,(void *)arg->p1);
       break;
    default:
       break;
  }
}
/*
* Passa a executar a rotina do kernel com interruo��es desabiitadas
*
*/

void callsvc(Parameters *args)
{
  noInterrupts();
  kernelargs = *args ;
  kernel() ;
  interrupts();
}

/*
*
*
*/
void saveContext(TaskDescriptor* task) {
    asm volatile (
        "push r0 \n\t"
        "in r0, __SREG__ \n\t"
        "cli \n\t"
        "push r0 \n\t"
        "push r1 \n\t"
        "clr r1 \n\t"
        "push r2 \n\t"
        "push r3 \n\t"
        "push r4 \n\t"
        "push r5 \n\t"
        "push r6 \n\t"
        "push r7 \n\t"
        "push r8 \n\t"
        "push r9 \n\t"
        "push r10 \n\t"
        "push r11 \n\t"
        "push r12 \n\t"
        "push r13 \n\t"
        "push r14 \n\t"
        "push r15 \n\t"
        "push r16 \n\t"
        "push r17 \n\t"
        "push r18 \n\t"
        "push r19 \n\t"
        "push r20 \n\t"
        "push r21 \n\t"
        "push r22 \n\t"
        "push r23 \n\t"
        "push r24 \n\t"
        "push r25 \n\t"
        "push r26 \n\t"
        "push r27 \n\t"
        "push r28 \n\t"
        "push r29 \n\t"
        "push r30 \n\t"
        "push r31 \n\t"
        "in %A0, __SP_L__ \n\t"
        "in %B0, __SP_H__ \n\t"
        : "=r" (task->P)
    );
}

void restoreContext(TaskDescriptor* task) {
    asm volatile (
        "out __SP_L__, %A0 \n\t"
        "out __SP_H__, %B0 \n\t"
        "pop r31 \n\t"
        "pop r30 \n\t"
        "pop r29 \n\t"
        "pop r28 \n\t"
        "pop r27 \n\t"
        "pop r26 \n\t"
        "pop r25 \n\t"
        "pop r24 \n\t"
        "pop r23 \n\t"
        "pop r22 \n\t"
        "pop r21 \n\t"
        "pop r20 \n\t"
        "pop r19 \n\t"
        "pop r18 \n\t"
        "pop r17 \n\t"
        "pop r16 \n\t"
        "pop r15 \n\t"
        "pop r14 \n\t"
        "pop r13 \n\t"
        "pop r12 \n\t"
        "pop r11 \n\t"
        "pop r10 \n\t"
        "pop r9 \n\t"
        "pop r8 \n\t"
        "pop r7 \n\t"
        "pop r6 \n\t"
        "pop r5 \n\t"
        "pop r4 \n\t"
        "pop r3 \n\t"
        "pop r2 \n\t"
        "pop r1 \n\t"
        "pop r0 \n\t"
        "out __SREG__, r0 \n\t"
        "pop r0 \n\t"
        : : "r" (task->P)
    );
}
//aqui que deve ser implementada a variavel decrementar o tempo *tempodinamico
// void wakeUP() //acorda a task bloqueada a espera de passagem de tempo
// {
//   int i=1;
//   for(i=1;i<=NUM_TASKS; i++)
//   {
//     //sleep
//     if()
//     if(Descriptors[i].Time>0)
//     {
//       Descriptors[i].Time--;
//       if(Descriptors[i].Time <= 0 && Descriptors[i].State == BLOCKED)
//       {
//         Descriptors[i].State = READY;
//         InsertReadyList (i) ;
//       }
//     }
//   }
// }
void wakeUP() //acorda a task bloqueada a espera de passagem de tempo
{
  int i=1;
  for(i=1;i<=NUM_TASKS; i++)
  {
    //sleep
    if(Descriptors[i].Time>0)
    {
      Descriptors[i].Time--;
      if(Descriptors[i].Time <= 0 && Descriptors[i].State == BLOCKED)
      {
        Descriptors[i].State = READY;
        switchTask();
        InsertReadyList (i) ;
      }
    }
     Descriptors[i].period_dinamic--;
     if(Descriptors[i].period_dinamic==0)
     {
        if(Descriptors[i].State==BLOCKED)
        {
          Descriptors[i].State =READY;
          // switchTask();
        }
     }
    if( Descriptors[i].State==RUNNING)
    {
      Descriptors[i].time_computing_dinamic--;
        if(Descriptors[i].time_computing_dinamic ==0)
        {
          Descriptors[i].State==BLOCKED;
          // switchTask();
        } 
    }
  }

}

//Escalonador

 void switchTask2() {
    // Salva o contexto da tarefa atual
    saveContext(&Descriptors[TaskRunning]);
    // Atualiza TaskRunning para a pr�xima tarefa pronta
    do {
        ready_queue.head = (ready_queue.head + 1) % NumberTaskAdd ;
        TaskRunning = ready_queue.queue[ready_queue.head];
    } while (Descriptors[TaskRunning].State != READY && ready_queue.head != ready_queue.tail);

    // Se encontrou uma tarefa pronta, muda o estado para RUNNING e restaura o contexto
   // if (Descriptors[TaskRunning].State == READY) {        
        Descriptors[TaskRunning].State = RUNNING;
        restoreContext(&Descriptors[TaskRunning]);
    //}
 }

void switchTask() {
  saveContext(&Descriptors[TaskRunning]) ; //nao mexe
  TaskRunning = (TaskRunning + 1) % NumberTaskAdd ;
  // // chama buble short
  // bubbleSortReadyQueue();
  while(Descriptors[TaskRunning].State == BLOCKED) {
     TaskRunning = (TaskRunning + 1) % NUM_TASKS ;

  }
  restoreContext(&Descriptors[TaskRunning]); // nao mexe
}
void bubbleSortReadyQueue() {
    int n = (ready_queue.tail >= ready_queue.head) ? 
            ready_queue.tail - ready_queue.head : 
            MaxNumberTask - ready_queue.head + ready_queue.tail;

    for (int i = 0; i < n - 1; i++) {
        for (int j = ready_queue.head; j != ready_queue.tail; j = (j + 1) % MaxNumberTask) {
            int next = (j + 1) % MaxNumberTask;

            // Compara `time_computing_dinamic` das tarefas na fila para ordenação
            if (Descriptors[ready_queue.queue[j]].time_computing_dinamic > Descriptors[ready_queue.queue[next]].time_computing_dinamic) {
                // Troca os índices das tarefas na fila
                int temp = ready_queue.queue[j];
                ready_queue.queue[j] = ready_queue.queue[next];
                ready_queue.queue[next] = temp;
            }
        }
    }
}

//Trata a interrup��o do Timer

void systemContext() { //Chamada pela interrup��o do Timer
  wakeUP(); //decrementa o tempo na wakeup
  switchTask();
}

/*
*
* Idle Process
*
*/
void idle() {
     while (1) {
     } ;
}


/*
*
* Sys Call
*
*/
// void sys_taskcreate(int *tid, void (*taskFunction)(void)) {
void sys_taskcreate(int *tid, void (*taskFunction)(void), unsigned short time_computing, unsigned short period) {

    NumberTaskAdd++ ;
    *tid = NumberTaskAdd;
    Descriptors[NumberTaskAdd].Tid=*tid;
    Descriptors[NumberTaskAdd].State=READY;
    Descriptors[NumberTaskAdd].Join=0;
    Descriptors[NumberTaskAdd].Time=0 ;
    Descriptors[NumberTaskAdd].Prio=0;
    Descriptors[NumberTaskAdd].time_computing_dinamic=time_computing;
    Descriptors[NumberTaskAdd].period_dinamic=period;
    Descriptors[NumberTaskAdd].time_computing_static=time_computing;
    Descriptors[NumberTaskAdd].period_static=period;
    uint8_t* stack = Descriptors[*tid].Stack + SizeTaskStack - 1;
    Descriptors[*tid].P = stack;

    *(stack--) = ((uint16_t)taskFunction) & 0xFF; // PC low byte
    *(stack--) = ((uint16_t)taskFunction >> 8) & 0xFF; // PC high byte
    *(stack--) = 0x00; // R0
    *(stack--) = 0x80; // SREG with global interrupts enabled

    for (int i = 1; i < 32; i++) {
        *(stack--) = i; // Initialize all other registers with their number
    }

    Descriptors[*tid].P = stack;
}

void InsertReadyList(int id) {
   // nkprint("Insert: ", 0);
    switch (SchedulerAlgorithm) {
        case RR:
            if (ready_queue.head == ready_queue.tail && ready_queue.queue[ready_queue.head] == INVALID_INDEX ) {  // Fila vazia
               {
                ready_queue.tail = 0 ;
                ready_queue.head = 0 ;
                ready_queue.queue[ready_queue.head] = id ;
               }  
            } else {
                // Atualiza o tail circularmente
                ready_queue.tail = (ready_queue.tail + 1) % NumberTaskAdd;
                ready_queue.queue[ready_queue.tail] = id;
                Descriptors[id].State == READY ;
            }
            break;
        default:
            break;
    }
}

void sys_start(int scheduler) {
    int i;
    SchedulerAlgorithm = scheduler;
    switch (SchedulerAlgorithm) {
        case RR:
            for (i = 0; i <= NumberTaskAdd; i++) {
                InsertReadyList(i);
            }
            break;
        case EDF:
            Serial.println("esta no edf");
            for (i = 0; i <= NumberTaskAdd; i++) {
                InsertReadyList(i);
            }
            break;
        default:
            break;
    }
}

void sys_getmynumber(int *number)
{
  *number=Descriptors[TaskRunning].Tid ;
}

void sys_ligaled()
{
  PORTB = PORTB | 0x20;
}

void sys_desligaled()
{
  PORTB = PORTB & 0xDF;
}

void sys_setmyname(const char *name)
{
  Descriptors[TaskRunning].name=name;
}

void sys_getmyname(const char *name)
{
  strcpy(name, Descriptors[TaskRunning].name);
}

void sys_semwait(sem_t *semaforo)
{   
    semaforo->count--;
    if(semaforo->count < 0)
    {
      semaforo->sem_queue[semaforo->tail] = TaskRunning;
      Descriptors[TaskRunning].State = BLOCKED ;
      semaforo->tail++;
      if(semaforo->tail == MaxNumberTask-1) semaforo->tail = 0;
      switchTask();
    }
}

void sys_sempost(sem_t *semaforo)
{
     semaforo->count++;
     if(semaforo->count <= 0)
     {
       Descriptors[semaforo->sem_queue[semaforo->header]].State = READY;
       InsertReadyList(semaforo->sem_queue[semaforo->header]);
       semaforo->header++;
       if(semaforo->header == MaxNumberTask-1) semaforo->header = 0;
     }
}

void sys_seminit(sem_t *semaforo, int ValorInicial)
{
  semaforo->count = ValorInicial;
  semaforo->header = 0;
  semaforo->tail = 0;
}

void sys_sleep(unsigned int segundo)
{
  //Descriptors[TaskRunning].Time = segundo/ClkT;
  Descriptors[TaskRunning].Time = (segundo*1000000)/Slice ;
  if(Descriptors[TaskRunning].Time > 0)
  {
    Descriptors[TaskRunning].State = BLOCKED;
    switchTask();
  }
}
void sys_msleep(unsigned int mili)
{
  Descriptors[TaskRunning].Time = (mili/ClkT)/1000;
  if(Descriptors[TaskRunning].Time > 0)
  {
    Descriptors[TaskRunning].State = BLOCKED;
    switchTask();
  }
}

void sys_usleep(unsigned int micro)
{
  Descriptors[TaskRunning].Time = (micro/ClkT)/1000000;
  if(Descriptors[TaskRunning].Time > 0)
  {
    Descriptors[TaskRunning].State = BLOCKED;
    switchTask();
  }
}

/*
*  calcularPrecisao( float valor) chamada pela sys_nkprint
*/
static inline int calcularPrecisao( float valor)
{
  int PRECISAO_FLOAT_ARDUINO = 6;
  int precisao = 0;
  int valorInteiro = (int)valor;
  while(valorInteiro > 0)
  {
    valorInteiro = valorInteiro / 10;
    precisao++;
  }
  return PRECISAO_FLOAT_ARDUINO - precisao;
}
void sys_nkprint(char *fmt,void *number)
{
  int *auxint;
  float *auxfloat;
  char *auxchar;
  int size=0;
  int accuracy=1;
  
  while (*fmt)
  {
    switch(*fmt)
      {
        case '%':
          fmt++;
          switch(*fmt)
            {
              case '%':
                Serial.print(*fmt);
                break;
              case 'c':
                Serial.print((char *)number);
                break;
              case 's':
                sys_nkprint((char *)number,0);
                break;
              case 'd':
                auxint=number;
                Serial.print(*auxint);
                break;
              case 'f':
                auxfloat=number;
                int precisao = calcularPrecisao(*auxfloat);
                Serial.print(*auxfloat, precisao);
                break;
              // case '.':
              //   fmt++;
              //   while(*fmt != 'f')
              //   {
              //     size*=accuracy;
              //     size+= (*fmt - '0');
              //     accuracy*=10;
              //     fmt++;
              //   }
              //   auxfloat=number;
              //   // printfloat(*auxfloat,size);
              //   break;
              // case 'x':
              //   auxint=number;
              //   // printhexL(*auxint);
              //   break;
              // case 'X':
              //   auxint=number;
              //   // printhexU(*auxint);
              //   break;
              // case 'b':
              //   fmt++;
              //   switch(*fmt)
              //   {
              //     case 'b':
              //       size=8;
              //       break;
              //     case 'w':
              //       size=16;
              //       break;
              //     case 'd':
              //       size=32;
              //       break;
              //     default:
              //       fmt -= 2;
              //       size = 32;
              //       break;
              //   }
              //   auxint=number;
              //   // printbinary(*auxint, size);
              //   break;
              default:
                break;
            }
        break;
        case '\\':
          fmt++;
          if (*fmt == 'n')
          {
            Serial.print("\n");
          }
          else
          {
            Serial.print("\\");
          }
        break;
        default:
          Serial.print(*fmt);
        break;
      }
    fmt++;
  }
}

void sys_taskexit(void)
{
  Descriptors[TaskRunning].State=BLOCKED;
  switchTask();
}
/*
*
* User Call
*
*/
void taskcreate(int *ID, void (*funcao)(), unsigned short time_computing, unsigned short period) {
    Parameters arg;
    arg.CallNumber = TASKCREATE;
    arg.p0 = (unsigned char *)ID;
    arg.p1 = (unsigned char *)funcao;
    arg.p2 = (unsigned char *)time_computing;  // Adiciona o tempo de computação dinâmico
    arg.p3 = (unsigned char *)period;          // Adiciona o período dinâmico
    callsvc(&arg);  // Chama o SVC
}
void start(int scheduler)
{
  Parameters arg;
  arg.CallNumber=START;
  arg.p0=(unsigned char *)scheduler;
  callsvc(&arg);
}
void semwait(sem_t *semaforo)
{
  Parameters arg;
  arg.CallNumber=SEM_WAIT;
  arg.p0=(unsigned char *)semaforo;
  callsvc(&arg);
}

void sempost(sem_t *semaforo)
{
  Parameters arg;
  arg.CallNumber=SEM_POST;
  arg.p0=(unsigned char *)semaforo;
  callsvc(&arg);
}

void seminit(sem_t *semaforo, int ValorInicial)
{
  Parameters arg;
  arg.CallNumber=SEM_INIT;
  arg.p0=(unsigned char *)semaforo;
  arg.p1=(unsigned char *)ValorInicial;
  callsvc(&arg);
}
void setmyname(const char *name)
{
  Parameters arg;
  arg.CallNumber=SETMYNAME;
  arg.p0=(unsigned char *)name;
  callsvc(&arg);
}
void getmynumber(int *number)
{
  Parameters arg;
  arg.CallNumber=GETMYNUMBER;
  arg.p0=(unsigned char *)number;
  callsvc(&arg);
}
void getmyname(const char *name)
{
  Parameters arg;
  arg.CallNumber=GETMYNAME;
  arg.p0=(unsigned char *)name;
  callsvc(&arg);
}
void sleep(int time)
{
  Parameters arg;
  arg.CallNumber=SLEEP;
  arg.p0=(unsigned char *)time;
  callsvc(&arg);
}
void msleep(int time)
{
  Parameters arg;
  arg.CallNumber=MSLEEP;
  arg.p0=(unsigned char *)time;
  callsvc(&arg);
}

void usleep(int time)
{
  Parameters arg;
  arg.CallNumber=USLEEP;
  arg.p0=(unsigned char *)time;
  callsvc(&arg);
}
void taskexit(void)
{
  Parameters arg;
  arg.CallNumber=EXITTASK;
  callsvc(&arg);
}

void ligaled(void)
{
  Parameters arg;
  arg.CallNumber=LIGALED;
  callsvc(&arg);
}

void desligaled(void)
{
  Parameters arg;
  arg.CallNumber=DESLIGALED;
  callsvc(&arg);
}

void nkprint(char *fmt,void *number)
{
  Parameters arg;
  arg.CallNumber=NKPRINT;
  arg.p0=(unsigned char *)fmt;
  arg.p1=(unsigned char *)number;
  callsvc(&arg);
}

/*
*
* Codigo Aplicacao
*
*/
volatile int  i, j ;
volatile unsigned long valor = 0 ;
volatile int16_t tid0, tid1, tid2, tid3, tid4;
sem_t s0;
sem_t s1;
sem_t s2;
sem_t s3;

void p0() {     
  static int number0;
  getmynumber(&number0);  
  while (1) {
    sleep(5);
    nkprint("P0: ", 0);
  }
}

void p1() {
  static int number1 ;
  getmynumber(&number1);
  while (1) {
    sleep(10);
    nkprint("P1: ", 0);
  }
}

void p2() {
  static int number2;
  getmynumber(&number2);
  while (1) {
    sleep(20);
    nkprint("P2: ", 0);
  }
}

void setup() {
    Serial.begin(117200) ;
    nkprint("FakeOS \n", 0) ;
    nkprint("Versao 0.0 \n", 0) ;
  
    //ligaled();

    //seminit(&s0, 1);
    //seminit(&s1, 0);
    //seminit(&s2, 0);
    //seminit(&s3, 0);

    //desligaled();

    ready_queue.head = INVALID_INDEX ;
    ready_queue.tail = INVALID_INDEX ; //fila vazia

    // taskcreate(&tid0,idle);
    // taskcreate(&tid1,p0);
    // taskcreate(&tid2,p1);
    // taskcreate(&tid3,p2);
    taskcreate(&tid0, idle, 20, 1); // Tarefa Idle: Tempo de computação 10, Período 20
    taskcreate(&tid1, p0, 5, 2);      // Tarefa p0: Tempo de computação 2, Período 5
    taskcreate(&tid2,p1,10,5);
    taskcreate(&tid3,p2,15,25);
    start (EDF) ; //coloca as tasks na fila
    noInterrupts(); 
    Timer1.initialize(Slice);
    Timer1.attachInterrupt(systemContext);
    restoreContext(&Descriptors[0]);
}

void loop() {
  while (1);
}
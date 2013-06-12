#Uncomment to add Multi Processor affinity support
#FLAGS=-DMULTIPROC
FLAGS=-Wall
#SLACK_STEALING=-DSLACK_STEALING

all:  task-example task-err task-sp task-sp-err task-sp-err-wcet

task-example: main.o executive.o task-example.o excstate.o
	gcc ${FLAGS} -o test-task-example main.o executive.o task-example.o excstate.o -lpthread -lrt

task-err: main.o executive.o task-err.o excstate.o
	gcc ${FLAGS} -o test-task-err main.o executive.o task-err.o excstate.o -lpthread -lrt

task-sp: main.o executive.o task-sp.o excstate.o
	gcc ${FLAGS} -o test-task-sp main.o executive.o task-sp.o excstate.o -lpthread -lrt

task-sp-err: main.o executive.o task-sp-err.o excstate.o
	gcc ${FLAGS} -o test-task-sp-err main.o executive.o task-sp-err.o excstate.o -lpthread -lrt

task-sp-err-wcet: main.o executive.o task-sp-err-wcet.o excstate.o
	gcc ${FLAGS} -o test-task-sp-err-wcet main.o executive.o task-sp-err-wcet.o excstate.o -lpthread -lrt

main.o: main.c
	gcc ${FLAGS} -Wall -c main.c -lpthread -lrt

executive.o: executive.c executive.h
	gcc ${FLAGS} ${SLACK_STEALING} -Wall -c executive.c -lpthread -lrt

excstate.o: excstate.c excstate.h
	gcc ${FLAGS} -Wall -c excstate.c -lpthread -lrt

task-example.o: task-example.c
	gcc ${FLAGS} -Wall -c task-example.c -lpthread -lrt

task-err.o: task-err.c
	gcc ${FLAGS} -Wall -c task-err.c -lpthread -lrt

task-sp.o: task-sp.c
	gcc ${FLAGS} -Wall -c task-sp.c -lpthread -lrt
	
task-sp-err.o: task-sp-err.c
	gcc ${FLAGS} -Wall -c task-sp-err.c -lpthread -lrt

task-sp-err-wcet.o: task-sp-err-wcet.c
	gcc ${FLAGS} -Wall -c task-sp-err-wcet.c -lpthread -lrt
	
clean:
	rm -f *.o *~ test-*


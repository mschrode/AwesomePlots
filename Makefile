ROOTLIBS   = $(shell root-config --libs)
ROOTCFLAGS = $(shell root-config --cflags)

CFLAG      = -I $(ROOTCFLAGS)
LFLAG      = $(ROOTLIBS)

OBJ     = Config.o DataSet.o Event.o EventBuilder.o EventInfoPrinter.o EventYieldPrinter.o Filter.o GlobalParameters.o MrRA2.o Output.o PlotBuilder.o Selection.o Style.o Variable.o



run: $(OBJ)		
	g++ $(OBJ) $(LFLAG) -o run
	@echo -e 'Done.\n\n   Type "./run config-file-name" and let MrRA2 amaze you.\n\n'

Config.o: Config.h Config.cc
	g++ $(CFLAG) -c  Config.cc

DataSet.o: DataSet.h DataSet.cc Config.h Event.h GlobalParameters.h Selection.h Variable.h
	g++ $(CFLAG) -c  DataSet.cc

Event.o: Event.h Event.cc Variable.h
	g++ $(CFLAG) -c  Event.cc

Filter.o: Filter.h Filter.cc Config.h Event.h GlobalParameters.h Selection.h Variable.h
	g++ $(CFLAG) -c  Filter.cc

EventBuilder.o: EventBuilder.h EventBuilder.cc Event.h Variable.h
	g++ $(CFLAG) -c  EventBuilder.cc

EventInfoPrinter.o: EventInfoPrinter.h EventInfoPrinter.cc Config.h DataSet.h Event.h Output.h Selection.h Variable.h
	g++ $(CFLAG) -c  EventInfoPrinter.cc

EventYieldPrinter.o: EventYieldPrinter.cc EventYieldPrinter.h DataSet.h Output.h Selection.h Style.h
	g++ $(CFLAG) -c EventYieldPrinter.cc

GlobalParameters.o: GlobalParameters.h GlobalParameters.cc Config.h
	g++ $(CFLAG) -c  GlobalParameters.cc

MrRA2.o: MrRA2.h MrRA2.cc DataSet.h Config.h GlobalParameters.h PlotBuilder.h Selection.h EventInfoPrinter.h EventYieldPrinter.h Output.h Style.h Variable.h
	g++ $(CFLAG) -c  MrRA2.cc

Output.o: Output.h Output.cc GlobalParameters.h 
	g++ $(CFLAG) -c Output.cc

PlotBuilder.o: PlotBuilder.h PlotBuilder.cc DataSet.h Variable.h Config.h GlobalParameters.h Event.h Output.h Selection.h Style.h
	g++ $(CFLAG) -c  PlotBuilder.cc

Selection.o: Selection.h Selection.cc Config.h Event.h Filter.h
	g++ $(CFLAG) -c  Selection.cc

Style.o: Style.h Style.cc Config.h DataSet.h Selection.h
	g++ $(CFLAG) -c  Style.cc

Variable.o: Variable.h Variable.cc Config.h
	g++ $(CFLAG) -c  Variable.cc



clean:
	@rm -f *.o 
	@rm -f run
	@rm -f *~
	@rm -f *#
	@rm -f .#*



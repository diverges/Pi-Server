JFLAGS = -g
JC = javac
JVM = java
JLOC = -d bin -sourcepath src

#
#   Macro for each of the java source files.
#
CLASSES = \
	Client.java

# Dependencies in lib/ 
 JDEP = -cp \
 	bin

# Name of class with main function
MAIN = Client

#
#
#
default: classes

classes: 
	$(JC) $(JFLAGS) $(JLOC) $(JDEP) src/$(CLASSES)

run: classes
	$(JVM) $(JDEP) $(MAIN)

clean:
	$(RM)  bin/*.class

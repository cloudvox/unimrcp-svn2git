#! /bin/sh

find_java() {
	if ! [ -z "$JAVA" ] && [ -x "$JAVA" ]; then
		return 0;
	fi;
	JAVA=`which java`;
	if ! [ -z "$JAVA" ] && [ -x "$JAVA" ]; then
		return 0;
	fi;
	if ! [ -z "$JAVA_HOME" ]; then
		if [ -x "$JAVA_HOME/bin/java" ]; then
			JAVA="$JAVA_HOME/bin/java";
			return 0;
		fi;
	fi;
	return 1;
}

find_javac() {
	if ! [ -z "$JAVAC" ] && [ -x "$JAVAC" ]; then
		return 0;
	fi;
	JAVAC=`which javac`;
	if ! [ -z "$JAVAC" ] && [ -x "$JAVAC" ]; then
		return 0;
	fi;
	if ! [ -z "$JAVA_HOME" ]; then
		if [ -x "$JAVA_HOME/bin/javac" ]; then
			JAVAC="$JAVA_HOME/bin/javac";
			return 0;
		fi;
	fi;
	return 1;
}

if ! find_java; then
	echo "Java not found.";
	echo "If installed, add it to the PATH or set JAVA variable.";
	exit 1;
fi;

if ! find_javac; then
	echo "Java compiler not found.";
	echo "If installed, add it to the PATH or set JAVAC variable.";
	exit 1;
fi;

echo "Java runtime: $JAVA";
echo "Using Java compiler: $JAVAC";
read -p "Press enter to compile UniSynth" x;
$JAVAC UniSynth.java wrapper/*.java
if [ $? -ne 0 ]; then
	echo "Error compiling UniSynth";
	exit $?;
fi;

read -p "Press enter to run UniSynth $*" x;
$JAVA -classpath .:wrapper -Djava.library.path=. UniSynth $*

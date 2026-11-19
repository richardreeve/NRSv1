export CLASSPATH="$CLASSPATH:$PWD/src/"

echo 'compiling Java files'
echo '*************************'
echo 'compiling GUI'
javac -deprecation src/main_method/GUI.java -d bin/
echo 'misc'
javac -deprecation src/misc/*.java -d bin/
echo 'gui component classes'
javac -deprecation src/gui_stuff/*.java -d bin/
echo 'internal variable classes'
javac -deprecation src/variable_stuff/*.java -d bin/
echo 'link terminal container classes'
javac -deprecation src/world_objects/*.java -d bin/
echo 'xml parser'
javac -deprecation src/xml_parser/helpers/*.java -d bin/
javac -deprecation src/xml_parser/*.java -d bin/
echo ''
echo 'Finished'
#clean up
#rm *~

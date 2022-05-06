echo Compile + compile_commands.json..
bear -- make
if [ $? = 0 ]; then
    echo Running..
    cd bin/; ./dev; cd ..
    echo Done!
else
    echo Not running.
fi


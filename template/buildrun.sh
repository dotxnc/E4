if [ "$1" == "--engine" ]; then
    echo Building engine..
    echo ----------------------------------------
    cwd=`pwd`
    cd ../E4/
    make
    if [ $? = 2 ]; then
        echo Engine build failed, aborting..
        exit 1
    fi
    cd $cwd
    echo; echo
fi

echo Compile + compile_commands.json..
echo ----------------------------------------
bear -- make

if [ $? = 0 ]; then
    echo; echo
    echo Running..
    cd bin/; ./dev; cd ..
    echo Done!
else
    echo; echo
    echo Not running.
fi


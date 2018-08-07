valgrind --leak-check=yes --error-exitcode=1 ./cli <<< `echo -e "exit\n"`

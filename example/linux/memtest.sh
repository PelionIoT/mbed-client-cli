printf "help && exit\n" | valgrind --leak-check=yes --error-exitcode=1 ./cli

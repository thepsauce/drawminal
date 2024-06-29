#!/bin/bash

projects=()

project_file=
project_name=

project_write=false

magic="__PROJECT_FILE=true"

choose_main=

source_dirs=()
sources=()
headers=()
rebuild=false
objects=()
main_objects=()
program=
do_execute=false
do_linking=false
do_debug=false

common_flags=()
compiler_flags=()
linker_libs=()
linker_flags=()

# 1. Check for project files
last_dir="$(pwd)"
until [ ${#projects[@]} -gt 0 ] || [ "$(pwd)" = "/" ]
do
    for f in .*
    do
        if [ ! -f "$f" ]
        then
            continue
        fi
        read -r line < "$f"
        if [ "$line" = "$magic" ]
        then
            projects+=("${f:1}")
        fi
    done
    \builtin cd .. || break
done
\builtin cd "$last_dir" || exit 1

# 2. Parse program arguments
cur=
arg_opts=(--init --project -o -t --test --source -s)
while [ -n "$cur" ] || [ $# -ne 0 ]
do
    # 2.1 Parse special arguments
    if [ -z "$cur" ]
    then
        cur="$1"
        shift
    fi
    flag=
    arg=
    arg_shift=false
    case "$cur" in
    --*=*)
        arg="${cur#*=}"
        flag="${cur%%=*}"
        cur=
        ;;
    --*)
        flag="$cur"
        arg="$1"
        cur=
        arg_shift=true
        ;;

    -f*)
        common_flags+=("$cur")
        cur=
        rebuild=true
        continue
        ;;
    -I*|-W*|-std=*)
        compiler_flags+=("$cur")
        cur=
        continue
        ;;
    -L*)
        linker_flags+=("$cur")
        cur=
        continue
        ;;
    -l*)
        linker_libs+=("$cur")
        cur=
        continue
        ;;

    -*)
        flag="${cur:0:2}"
        cur="${cur:2}"
        if [ -n "$cur" ]
        then
            cur="-$cur"
        else
            arg="$1"
            arg_shift=true
        fi
        ;;
    *)
        echo "invalid argument $cur" >&2
        exit 1
        ;;
    esac

    # 2.2 Shift argument if this flag expects one
    for a in "${arg_opts[@]}"
    do
        if $arg_shift && [ "$a" = "$flag" ]
        then
            [ $# = 0 ] && { echo "$flag expects an argument" >&2; exit ; }
            shift
            break
        fi
    done

    # 2.3 Parse flags
    case "$flag" in
    --init)
        for p in "${projects[@]}"
        do
            if [ "$p" = "$project_name" ]
            then
                echo "project $p already exists" >&2
                exit 1
            fi
        done
        case "$arg" in
        *:*)
            source_dirs+=("${arg#*:}")
            ;;
        esac
        case "$arg" in
        all*)
            mkdir -p src
            mkdir -p include
            mkdir -p tests
            mkdir -p lib
            echo -e "int main(void)\n{\n\treturn 0;\n}\n" > src/main.c
            source_dirs=(src include)
            ;;
        simple*)
            mkdir -p src
            mkdir -p tests
            echo -e "int main(void)\n{\n\treturn 0;\n}\n" > src/main.c
            source_dirs=(src)
            ;;
        none*)
            ;;
        *)
            echo "invalid argument for --init (all|simple|none)" >&2
            exit 1
            ;;
        esac
        project_write=true
        compiler_flags+=(-Wall -Wextra -Werror -Ibuild)
        touch "$project_file"
        echo "project $project_name initialized!" >&2
        ;;

    -B)
        rebuild=true
        ;;
    -p|--project)
        project_name="$arg"
        project_file=".$arg"
        ;;
    -w|--write)
        project_write=true
        ;;
    -s|--source)
        source_dirs+=("$arg")
        ;;
    --clean)
        rm -r build
        exit
        ;;
    -o)
        program="$arg"
        ;;
    -g|--debug)
        do_debug=true
        ;;
    -t|--test)
        [ -f "tests/$arg.c" ] || { echo "test file tests/$arg.c does not exist" ; exit 1 ; }
        choose_main="build/tests/$arg.o"
        sources+=("tests/$arg.c")
        program="build/tests/$arg"
        ;;
    --trace)
        set -o xtrace
        ;;
    -x|--execute)
        do_execute=true
        ;;
    --)
        break
        ;;
    *)
        echo "invalid argument $1" >&2
        exit
    esac
done

# 3. Choose a project
if [ -z "$project_file" ]
then
    if [ ${#projects[@]} -eq 0 ]
    then
        echo "no active projects, use --project <name> --init simple" >&2
        exit 1
    fi
    if [ ${#projects[@]} -eq 1 ]
    then
        project_file=".${projects[0]}"
    else
        select s in "${projects[@]}"
        do
            [ -z "$s" ] && { echo cancelled >&2 ; exit ; }
            project_file=".$s"
            break
        done
    fi
fi

# 4. Read project file
source "$project_file" >/dev/null 2>/dev/null || { echo "could not source $project_file" >&2 ; exit 1 ; }

source_dirs+=("${project_source_dirs[@]}")
common_flags+=("${project_common_flags[@]}")
linker_libs+=("${project_linker_libs[@]}")

if [ ${#common_flags[@]} -gt 0 ]
then
    readarray -t common_flags < <(printf '%s\n' "${common_flags[@]}" | sort -u)
fi

if [ ${#source_dirs[@]} -gt 0 ]
then
    readarray -t source_dirs < <(printf '%s\n' "${source_dirs[@]}" | sort -u)
fi

if [ ${#linker_libs[@]} -gt 0 ]
then
    readarray -t linker_libs < <(printf '%s\n' "${linker_libs[@]}" | sort -u)
fi

# 5. Write project file (if requested or the project was just initialized)
if $project_write
then
    {
        echo "$magic"
        echo "project_name=\"$project_name\""

        echo -n "project_source_dirs=( "
        for d in "${source_dirs[@]}"
        do
            echo -n "\"${d/\"/\\\"}\" "
        done
        echo ")"

        echo -n "project_common_flags=( "
        for d in "${common_flags[@]}"
        do
            echo -n "\"${d/\"/\\\"}\" "
        done
        echo ")"

        echo -n "project_linker_libs=( "
        for d in "${linker_libs[@]}"
        do
            echo -n "\"${d/\"/\\\"}\" "
        done
        echo ")"
    } > "$project_file"
fi

# 6. Find sources and headers
for dir in "${source_dirs[@]}"
do

    while read -r -d $'\0' f
    do
        case "$f" in
        *.c)
            sources+=("$f")
            ;;
        *.h)
            headers+=("$f")
            ;;
        esac
    done < <(\find "$dir" -type f '(' -name '*.c' -o -name '*.h' ')' -print0)
done

# 7. Compile
if [ -z "$program" ]
then
    program="build/$project_name"
fi

mkdir -p build/tests build/src || exit 1

compiler_flags+=("${common_flags[@]}")
linker_flags+=("${common_flags[@]}")

if [ -f "src/$project_name.h" ]
then
    for h in "${headers[@]}"
    do
        if [ "src/$project_name.h" -nt "build/$project_name.h.gch" ] ||
            [ "$h" -nt "build/$project_name.h.gch" ]
        then
            gcc "${compiler_flags[@]}" "src/$project_name.h" -o "build/$project_name.h.gch" || exit 1
            break
        fi
    done
fi

# 7.1 This checks what c files include what header files (used to check if a
# c file needs recompiling)
checked=()
chk_headers() {
    file=$1
    obj=$2
    while IFS= read -r line
    do
        line="${line## }"
        case "$line" in
        '#include "'*)
            local found
            line="src/${line:10:-1}"
            found=false
            for c in "${checked[@]}"
            do
                if [ "$c" = "$line" ]
                then
                    found=true
                    break
                fi
            done
            $found && checked+=("$line")
            $found || if [ "$obj" -ot "$line" ] ||
                chk_headers "$line" "$obj"
            then
                return 0
            fi
            ;;
        '#'*|'')
            ;;
        *)
            return 1
            ;;
        esac
    done < "$file"
    return 1
}

for s in "${sources[@]}"
do
    o="build/${s:0:-2}.o"
    checked=()
    l=
    [ "$s" -nt "$o" ] && l="u"
    [ -z "$l" ] && $rebuild && l="r"
    [ -z "$l" ] && chk_headers "$s" "$o" && l="h"
    if [ -n "$l" ]
    then
        echo +$l gcc "${compiler_flags[@]}" -c "$s" -o "$o" >&2
        gcc "${compiler_flags[@]}" -c "$s" -o "$o" || exit 1
        do_linking=true
    fi
    if nm "$o" | grep -q ' T main'
    then
        main_objects+=("$o")
    else
        objects+=("$o")
    fi
done

if [ -z "$choose_main" ]
then
    if [ ${#main_objects[@]} -eq 0 ]
    then
        echo no main function exists >&2
        exit 1
    elif [ ${#main_objects[@]} -eq 1 ]
    then
        objects+=("${main_objects[0]}")
    else
        select o in "${main_objects[@]}"
        do
                [ -z "$o" ] && { echo cancelled >&2 ; exit 1; }
                objects+=("$o")
                do_linking=true
                break
        done
    fi
else
    objects+=("$choose_main")
fi

if $do_linking || [ ! -f "$program" ]
then
    gcc "${linker_flags[@]}" "${objects[@]}" -o "$program" "${linker_libs[@]}" || exit 1
fi

# 8. Run the program (either with the debugger or normal as requested)
signal_handler() {
    echo "Caught SIGINT!" >&2
}

if $do_debug
then
    gdb --args ./"$program" "$@"
elif $do_execute
then
    read -r start_seconds start_nanoseconds < <(date "+%s %N")
    trap signal_handler SIGINT
    ./"$program" "$@"
    exit_code=$?
    read -r end_seconds end_nanoseconds < <(date "+%s %N")
    diff_seconds=$((10#$end_seconds - 10#$start_seconds))
    diff_nanoseconds=$((10#$end_nanoseconds - 10#$start_nanoseconds))
    if [ $diff_nanoseconds -lt 0 ]
    then
        diff_seconds=$((diff_seconds - 1))
        diff_nanoseconds=$((1000000000 + diff_nanoseconds))
    fi
    echo -e "\nexit code: \e[36m$exit_code\e[0m; elapsed time: \e[36m$diff_seconds.$diff_nanoseconds\e[0m seconds" >&2
fi

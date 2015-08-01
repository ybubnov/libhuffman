#!/bin/bash -e

_time()
{
    local -i nanoseconds=$(date +%s%N)
    local -i milliseconds=$(( ${nanoseconds} / 1000000 ))

    echo "${milliseconds}"
}

_error()
{
    echo "$@" >&2
}

_usage()
{
    local script_name = ${BASH_SOURCE[0]}

    _error "usage:"
    _error "  ${script_name} -f FILE"
}


while getopts f: option; do
    case ${option} in 
        f)
            INPUT_FILE=${OPTARG}
            ;;
        \?)
            _error "invalid option: -${OPTARG}"
            _usage

            exit 1
            ;;
    esac
done

# Archiver binary file name
ARCHIVER=hrc

# Get start milliseconds
BEGINNING=$(_time)

${ARCHIVER} -c "${INPUT_FILE}" "${INPUT_FILE}.huf"
${ARCHIVER} -x "${INPUT_FILE}.huf" "${INPUT_FILE}".copy

# Get end milliseconds
END=$(_time)

echo "scale=10; (${END} - ${BEGINNING}) / 1000.0" | bc

md5sum "${INPUT_FILE}"
md5sum "${INPUT_FILE}".copy

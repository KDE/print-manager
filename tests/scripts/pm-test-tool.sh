#!/usr/bin/env bash

# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

ACTION=""
VERBOSE=0

usage() {
    cat <<EOF
Usage: $0 [OPTIONS] [ARGS...]

Actions (choose one):
  -c, --create num name     Create [num] printers with the prefix [name]
  -r, --remove name         Remove printers with the prefex [name]
  -s, --start [Destname]    Start IPPEVEPRINTER with [Destname] (default: TestPrinter)
  -p, --pause name          Pause (disable) printer [name]
  -m, --resume name         Resume (enable) printer [name]
  -l, --location name location         Set printer [name] location
  -d, --description name description   Set printer [name] description
  -f, --default name        Set printer [name] to the default printer

Options:
  -v, --verbose   Enable verbose output
  -h, --help      Show this help

Examples:
  $0 --create 2 pmtest
  $0 --remove pmtest
  $0 --start [TestPrinter]
EOF
}

error() {
    echo "Error: $*" >&2
    exit 1
}

log() {
    if [[ $VERBOSE -eq 1 ]]; then
        echo "[DEBUG] $*"
    fi
}

# Args
if [[ $# -eq 0 ]]; then
    usage
    exit 1
fi

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c|--create)
            [[ -n "$ACTION" ]] && error "Only one action allowed"
            ACTION="create"
            shift
            ;;

        -r|--remove)
            [[ -n "$ACTION" ]] && error "Only one action allowed"
            ACTION="remove"
            shift
            ;;

        -s|--start)
            [[ -n "$ACTION" ]] && error "Only one action allowed"
            ACTION="start"
            shift
            ;;

        -p|--pause)
            [[ -n "$ACTION" ]] && error "Only one action allowed"
            ACTION="pause"
            shift
            ;;

        -m|--resume)
            [[ -n "$ACTION" ]] && error "Only one action allowed"
            ACTION="resume"
            shift
            ;;

        -l|--location)
            [[ -n "$ACTION" ]] && error "Only one action allowed"
            ACTION="location"
            shift
            ;;
   
        -d|--description)
            [[ -n "$ACTION" ]] && error "Only one action allowed"
            ACTION="description"
            shift
            ;;
   
        -f|--default)
            [[ -n "$ACTION" ]] && error "Only one action allowed"
            ACTION="default"
            shift
            ;;
            
        -v|--verbose)
            VERBOSE=1
            shift
            ;;

        -h|--help)
            usage
            exit 0
            ;;

        --) # end of options
            shift
            break
            ;;

        -*)
            error "Unknown option: $1"
            ;;

        *)
            break
            ;;
    esac
done

# Remaining args
ARGS=("$@")

[[ -z "$ACTION" ]] && error "You must specify --create, --remove, or --start"

case "$ACTION" in
    create)
        log "Creating printers with name: ${ARGS[*]}"
        if [[ ${ARGS[0]} -eq 1 ]]; then
            sudo lpadmin -h localhost:631 -p ${ARGS[1]} -D "My Test Printer" -E -v ipp://localhost:8000/ipp/print -m everywhere
        else
            for ((i = 0 ; i < ${ARGS[0]} ; i++)); do
                sudo lpadmin -h localhost:631 -p ${ARGS[1]}$i -D "My Test Printer $i" -E -v ipp://localhost:8000/ipp/print -m everywhere
                done
        fi
        ;;

    remove)
        [[ ${#ARGS[@]} -eq 0 ]] && error "Enter printer name prefix to remove"
        log "Removing printers that start with: ${ARGS[0]}"
        PR=$(ipptool -tv ipp://localhost/printers /usr/share/cups/ipptool/get-printers.test | grep printer-name | awk '{print $4}')
        for i in $PR; do
            if [[ $i == "${ARGS[0]}"* ]]; then
                log "$i startsWith ${ARGS[0]}, Removing"
                sudo lpadmin -x $i
                # sleep 1
            fi
            done
        ;;

    start)
        if [[ ${#ARGS[@]} -eq 0 ]]; then
            P="TestPrinter"
        else
            P=${ARGS[0]}
        fi
        
        log "Starting printer called: $P"
        ippeveprinter -v -f application/pdf,image/pwg-raster,image/jpeg -m PMTEST -l Downstairs $P &
        ;;
        
    pause)
        [[ ${#ARGS[@]} -eq 0 ]] && error "Enter printer name to pause"
        log "Pausing printer: ${ARGS[0]}"
        sudo cupsdisable ${ARGS[0]}
        ;;
        
    resume)
        [[ ${#ARGS[@]} -eq 0 ]] && error "Enter printer name to resume"
        log "Resuming printer: ${ARGS[0]}"
        sudo cupsenable ${ARGS[0]}
        ;;

    location)
        [[ ${#ARGS[@]} -eq 0 ]] && error "Enter printer name to set location"
        log "Setting location: ${ARGS[*]}"
        sudo lpadmin -p ${ARGS[0]} -L ${ARGS[1]}
        ;;
        
    description)
        [[ ${#ARGS[@]} -eq 0 ]] && error "Enter printer name to set description"
        log "Setting description ${ARGS[*]}"
        sudo lpadmin -p ${ARGS[0]} -D ${ARGS[1]}
        ;;

    default)
        [[ ${#ARGS[@]} -eq 0 ]] && error "Enter printer name to set default"
        log "Setting default printer ${ARGS[*]}"
        sudo lpadmin -d ${ARGS[0]}
        ;;
esac

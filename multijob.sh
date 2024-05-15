#!/bin/bash

# Έλεγχος εάν δόθηκαν αρχεία ως είσοδος
if [ $# -eq 0 ]; then
    echo "Usage: $0 <file1> <file2> ... <fileN>"
    exit 1
fi


# Για κάθε αρχείο
for file in "$@"; do
    # Αν υπάρχει το αρχείο
    if [ ! -f "$file" ]; then
        echo "File $file does not exist. Skipping."
        continue
    fi

    #Ένα new line στο τέλος του αρχείου γιατί δεν διάβαζε την τελευταία εντολή
    echo >> "$file"

    # Διάβασμα κάθε γραμμής
    while IFS= read -r task; do
        #echo "Submitting task: $task"
        # Execute 
        eval "$task"
    done < "$file"
done

echo "All tasks submitted successfully."

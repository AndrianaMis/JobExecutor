# System Programming 2024 Project

## JobExecutor

Η εφαρμογή JobExecutor έχει ως σκοπό την εκτέλεση εργασιών που λαμβάνει από ένα αρχείο εισόδου.
**Σημαντικό** : Το input file (συμπεριλαμβάνεται) πρέπει να ονομάζεται file.txt και οι γραμμές να ξεκινούν με ./jobCommander ή jobCommander, δηλαδή να είναι όλα τα commands της μορφής ./jobCommander issueJob job ή ./jobCommander setConcurrency κλπ. Δεν λαμβάνεται ως line argument (args) αλλά βρίσκεται ήδη στο directory και ανοίγεται από τον JobCommander.
Αρχεία όπως αυτά των test cases που περιέχουν και γραμμές όπως ls myFile.txt μπορούν να ληφθούν ως input μέσω του multijob.sh .

### Compilation 

```bash
make all
```

Για διαγραφή των object files που δημιουργήθηκαν:

```bash
make clean
```

### Execution

Οι τρόποι εκτέλεσης αυτής της εφαρμογής είναι οι εξής: 

1. Ξεκιναμε την εκτέλεση του jobExecutorServer από ένα bash termiinal, και την εκτέλεση του jobCommander από ένα δεύτερο bash terminal  ( 2 terminals)

Μπορούμε να εκτελέσουμε τον jobExecutorServer με 2 τρόπους (_make ExecutorSever_  ή  _./jobExecutorServer_ )

```bash
make ExecutorServer         
```

```c
./jobExecutorServer
```

και στο 2ο terminal μπορούμε να εκτελέσουμε τον jobCommander με 2 τρόπους (_make Commander_  ή  _./jobCommander_ )

```bash
make Commander                        
```

```c
./jobCommander
```



2. Εκτελούμε μόνο τον jobCommander , ο οποίος δημιουργεί και ενεργοποιεί τον jobExecutorServer  (1 terminal)

```bash
make Commander
```

## Description

Διαθέσιμες εντολές:

 - **'issueJob < job >'** : Εισάγεται η εργασία προς εκτέλεση στο σύστημα. Στον jobCommander επιτρέφεται η τριπλέτα της εργασίας.

 - **'setConcurrency < N >'** : Αλλάζει τον βαμθό παραλλήλιας, δηλαδή το μέγιστο αριθμό ενεργών εργασιών που μπορεί να εκτελεί η εφαρμογή.

 - **'stop < job_id >'** : Τερματίζει την εκτέλεση της εργασίας με jobID==job_id, ή την βγάζει από την αναμονή για εκτέλεση.

 - **'poll [ running , queued ]'** : Ανάλογα με το όρισμα , επιστέφει στον commander τις εργασίες που τρέχουν ή περιμένουν αντίστοιχα.

 - **'exit'** : Τερματίζει τη λειτουργία του jobExecutorServer.


Έχω φτιάξει ένα file.txt που λειτουργεί ως original input file στην εφαρμογη. Ο jobCommander, αν δεν του δωθούν args, παίρνει αυτό το input file με τις εργασίες και τις στέλνει job-by-job στον jobExecutorServer για εκτέλεση.
Οπότε αν τρέξουμε:

```c
make Commander
```

ή

```c
./jobCommander
```

Θα ξεκινήσει η εκτέλεση του jobCommander και θα στείλει στον jobExecutorServer όλες τις εργασίες που βρίσκονται στο file.txt.



Ο jobCommander μπορεί επίσης να λάβει line argument εντολή προς εκτέλεση. Με αυτόν τον τρόπο θα στείλει στον jobExecutorServer μόνο αυτή την εντολή και θα την εκτελέσει. Για παράδειγμα: 

```c
gcc -o progDelay progDelay.c
./jobCommander issueJob ./progDelay 5
```

### Details

Αυτή η υλοποίηση της εφαρμογής JobExecutor χρησιμοποιεί: **JobCommander** μέσω του οποίου ο χρήστης αλληλεπιδρά με την εφαρμογή, λαμβάνει τις εντολές από το input file, τις επεξεργάζεται , κάνει fork ένα παιδί για κάθε εντολή, στέλνει σήμα SIGUSR1 στον jobExecutorServer και γράφει την εντολή στο pipe. Αν δεν λάβει εντολές από το input file, σημαίνει πως έχει λάβει args , μια εντολή την οποία θα γράψει στο pipe για τον jobExecutorServer χωρις επεξεργασία. Ο jobCommander δημιουργεί με fork και exec* τον jobExecutorServer αν δεν ήταν ήδη ενεργός, κάτι το οποίο ελέγχεται ανάλογα με την ύπαρξη ή όχι του αρχείου JobExecutorServer.txt. 
**JobExecutorServer** ο οποίος αρχικά δημιουργεί το JobExecutorServer.txt. και περιμένει το σήμα SIGUSR1. Όταν δεχτεί αυτό το σήμα, ανοίγει το pipe για διάβασμα και αρχίζει να διαβάζει τις εντολές που αποστέλνει ο jobCommander, της αναθέτει την τριπλέτα <job, job_id, queue_position> όπου job είναι η εργασία (e.g. issueJob touch random.txt) , job_XX είναι ο αύξων αριθμός που αθξάνεται για κάθε διεργασία που δέχεται ο jobExecutorServer και queue_position είναι η θέση στην ουρα που μπήκε η εργασία.Προσθέτει την τριπλέτα στην ουρά.Έχοντας  την ουρά με τις εργασίες προς εκτέλεση, επεξεργάζεται κάθε element (τριπλέτα) της ουράς αυτής, ώστε να αναγνωρίσει τον τύπο της εργασίας και να διαλέξει την κατάλληλη συνάρτηση για την εκτέλεση (issueJob, setConcurrency , poll , stopJob, exit_).  Συγκεκριμένα, για εργασία με τύπο issueJob αρχικά προσθέτει στην λίστα με τα running jobs την τριπλέτα που θα γινει execute, δημιουργεί ένα copy της εργασίας (η οποία έχει τη μορφη cat myfiletxt , touch example.txt etc. χωρίς το πρόθεμα issueJob) και το κάνει split, μετράει τα args (arg_count) ώστε να κάνει  memory allocation. Μετά χωρίζει σε tokens το job και κάνει fork() και exec*() την εργασία. Ο jobExecutorServer
περιμένει μέχρι να τελείωσει το παιδί το execution. Τέλος ανοίγει το pipe, γράφει την τριπέτα , κλείνει το pipe και απελευθερώνει allocated memory. 

**ΟΙ ΣΥΝΑΡΤΉΣΕΙΣ ΓΙΑ ΕΚΤΈΛΕΣΗ ΤΩΝ ΕΡΓΑΣΊΩΝ ΒΡΊΣΚΟΝΤΑΙ ΣΤΟ modules/job_functions.c και περιέχουν αναλυτικά σχόλια.**

Επεξήγηση **queue implementation**: Η ουρά περιέχει στοιχεία tripl , δηλαδή δομής τριπλέτας της οποίας τα μέλη είναι τα στοιχεία που χρειαζόμαστε για την τριπλέτα (job, jobID, qu_pos). Έτσι, έχοντας ένα στοιχείο της ουρας, έχουμε πρόσβαση σε κάθε μέλος της τριπλέτας που του έχει ανατεθεί.

### Script description

**multijob.sh < file1 > < fil2 > ...** : To script αυτό λαμβάνει σαν είσοδο αρχεία τα οποία περιέχουν εργασίες προς εκτέλεση, μία ανά γραμμή. Αναθέτει ένα new line στο τέλος κάθε αρχείου για να διαβάζεται η τελευταία εντολή. Διαβάζει κάθε γραμμή του κάθε αρχείου.

Τρέχουμε:

```bash
./multijob.sh file1.txt
```

και εισάγονται οι εργασίες που περιέχει το file1 στο σύστημα. Εκτελούνται είτε μέσω του jobCommander -> jobExecutorServer αν η εργασία έχει τη μορφή _./jobCommander issueJob touch myFile.txt_  , είτε αυτόνομα αν έχει τη μορφή _ls myFile.txt_


## Σημαντικές υποσημειώσεις
  

Μπορεί να χρεαστεί να εκτελέσουμε:

```bash
make set_permissions
```
Για execution permission στα jobCommander , jobExecutorServer αλλά και σε όποιο άλλο αρχείο χρειαστεί με **cmod +x ...**


Στην υλοποίηση της ουράς δεν έχω λάβει υπόψιν το ότι η ουρά δεν πρέπει να έχει περιορισμό μεγέθους.

Το allJobsStop.sh δεν δουλεύει. Λόγω έλλειψης χρόνου δεν πρόλαβα να το υλοποιήσω σωστά.

Δεν έχω υλοποιήσει το συγκεκριμένο κομμάτι της εργασίας: _Κατά την ολοκλήρωση μιας διεργασίας-παιδί, ο πυρήνας του Linux αποστέλλει στην γονική της διεργασία το σήμα (signal) SIGCHLD. Ο server σας θα πρέπει να διαχειρίζεται αυτό το σήμα έτσι ώστε να ενημερωθεί για τον τερματισμό κάποιας διεργασίας-παιδί, Όταν λάβει το SIGCHLD, θα πρέπει να πάρει από την ουρά αναμονής την επόμενη εργασία και να τη θέσει υπό εκτέλεση (δεδομένου ότι υπάρχει εργασία διαθέσιμη)_

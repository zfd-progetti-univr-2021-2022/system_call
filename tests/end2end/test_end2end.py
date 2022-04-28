"""
TEST END TO END DEL PROGETTO

Eseguire il programma con una delle seguenti flag:

* '--stress-prod': esegui stress test sugli eseguibili in modalita' produzione
* '--stress-debug': esegui stress test sugli eseguibili in modalita' debug
* '--verify-prod': verifica che l'output degli eseguibili in mod. produzione siano corretti
* '--verify-debug': verifica che l'output degli eseguibili in mod. debug siano corretti
"""

import sys
import re
import psutil
import os
import subprocess
import shutil
import signal
import time

curr_dir = os.path.dirname(os.path.abspath(__file__))
test_area_dir = os.path.join(curr_dir, "test_area")
root_dir = os.path.join(curr_dir, "..", "..")
source_dir = os.path.join(root_dir, "src")
client_0_exe_src_path = os.path.join(source_dir, "client_0")
server_exe_src_path = os.path.join(source_dir, "server")
client_0_exe_test_path = os.path.join(test_area_dir, "client_0")
server_exe_test_path = os.path.join(test_area_dir, "server")

MSG_QUEUE = "message_queue"
SHARED_MEMORY = "shared_memory"
SEMAPHORE = "semaphore"


def compile_final():
    """
    Compila gli eseguibili in modalita' produzione.
    """
    os.chdir(source_dir)

    assert "Makefile" in os.listdir(), "Makefile not found"
    assert subprocess.call(["make"], shell=True) == 0, "Couldn't compile project: exit code != 0"

    os.chdir(curr_dir)


def compile_debug():
    """
    Compila gli eseguibili in modalita' debug.
    """
    os.chdir(root_dir)

    assert "Makefile" in os.listdir(), "Makefile not found"
    assert subprocess.call(["make"], shell=True) == 0, "Couldn't compile project: exit code != 0"

    os.chdir(curr_dir)


def copy_to_test_area():
    """
    Copia eseguibili nell'area di test
    """
    assert os.path.isfile(client_0_exe_src_path), "client_0 executable doesn't exist"
    assert os.path.isfile(server_exe_src_path), "server executable doesn't exist"

    shutil.copy(client_0_exe_src_path, test_area_dir)
    shutil.copy(server_exe_src_path, test_area_dir)


def find_sendme_files():
    """
    Trova i file che verranno inviati dal client verso il server.

    Un file viene inviato se ha dimensione inferiore o uguale a 4 KB e se il nome inizia con "sendme_"

    :return list: lista di percorsi di file
    """
    sendme_files = []
    for root, _dirs, files in os.walk(test_area_dir):
        for name in files:
            file_path = os.path.join(root, name)
            if name.startswith("sendme_") and os.path.getsize(file_path) <= (4 * 1024): # 4 KB
                sendme_files.append(file_path)

    return sendme_files


def divide_work_by_four(t_string):
    """
    Data una stringa di caratteri li suddivide in quattro parti
    di uguale dimensione (o quasi: le prime parti possono essere piu' grandi di uno rispetto all'ultima parte).

    :param str t_string: stringa da suddividere in quattro parti uguali
    :return list: contiene 4 stringhe contententi le 4 porzioni uguali
    """
    n_chars = len(t_string)
    chars_for_part = [n_chars//4, n_chars//4, n_chars//4, n_chars//4]

    i = 0
    while sum(chars_for_part) < n_chars:
        chars_for_part[i] += 1
        i += 1

    parts = []
    start = 0
    for i in range(0, 4):
        end = start + chars_for_part[i]
        part = t_string[start:end]
        start += chars_for_part[i]
        parts.append(part)

    return parts


def prepare_messages(file, parts):
    """
    Prepara i pezzi di output atteso ciascuno contenente indice, 
    file di origine del contenuto, mezzo di comunicazione attraverso cui e' arrivato il messaggio 
    e il contenuto del messaggio stesso.

    :param str file: percorso file
    :param list parts: parti che compongono il messaggio
    :return list: lista di 4 messaggi che contengono il contenuto di <file>
    """
    channels = ["FIFO1", "FIFO2", "MsgQueue", "ShdMem"]
    messages = []
    for i, part in enumerate(parts):
        message = "[Parte {}, del file {} spedita dal processo # tramite {}]\n{}\n\n".format(i+1, file, channels[i], part)
        messages.append(message)
        #print(message)
        #print("-"*50)
    return messages


def prepare_expected_outputs(t_sendme_files):
    """
    Prepara gli output attesi a partire da una lista di file da inviare.

    :param list t_sendme_files: lista di percorsi di file da inviare
    :return list: lista di dizionari con file in input e in output e il contenuto atteso nell'output
    """
    expected_messages = []

    for file in t_sendme_files:
        with open(file, "r") as f_in:
            file_content = f_in.read()
        parts = divide_work_by_four(file_content)
        message_parts = prepare_messages(file, parts)

        expected_message = ""
        for part in message_parts:
            expected_message += part

        expected_messages.append({"input_file": file, "output_file": file + "_out", "output": expected_message})
    
    return expected_messages


def clear_ipcs():
    """
    Richiama ipcrm -a per eliminare tutte le IPC attive.
    """
    subprocess.call(["ipcrm", "-a"])


def get_ipcs():
    """
    Recupera con il comando ipcs tutte le informazioni delle IPC.
    :return dict: Dizionario con informazioni su code dei messaggi, memorie condivise e semafori
    """
    p = subprocess.run(["ipcs"], capture_output=True)
    output = p.stdout.decode()

    ipcs_data = {
        MSG_QUEUE: [], 
        SHARED_MEMORY: [],
        SEMAPHORE: [],
    }

    listing_item = None
    for line in output.splitlines():
        #print(f"{line=}")
        if line.strip() == "":
            continue

        if line.strip("- ") == "Message Queues":
            listing_item = MSG_QUEUE
        elif line.strip("- ") == "Shared Memory Segments":
            listing_item = SHARED_MEMORY
        elif line.strip("- ") == "Semaphore Arrays":
            listing_item = SEMAPHORE
        else:
            ipc_data = parse_ipc_line(line, listing_item)

            if ipc_data is not None:
                ipcs_data[listing_item].append(ipc_data)
    
    return ipcs_data


def parse_ipc_line(line, type):
    """
    Data una riga di output del comando ipcs e il tipo di IPC 
    effettua il parsing dei dati contenuti nella riga.

    Restituisce None se la riga contiene l'indice della tabella.

    :param str line: riga su cui effettuare il parsing
    :param type: tipo di IPC contenuta in line
    :return (None|dict): None oppure Dizionario con i dati recuperati da <line>
    """
    if line.startswith("key"):
        return None

    data = line.strip().split(" ")
    data = [d for d in data if d.strip() != ""]

    if type == MSG_QUEUE:
        if len(data) != 6:
            raise ValueError("Unexpected number of outputs for msg queue: " + str(data))

        return {
            "key": data[0],
            "msqid": int(data[1]),
            "owner": data[2],
            "perms": data[3],
            "used-bytes": int(data[4]),
            "messages": int(data[5]),
        }
    
    elif type == SHARED_MEMORY:
        if len(data) not in  [6, 7]:
            raise ValueError("Unexpected number of outputs for shared memory: " + str(data))
        
        parsed_data = {
            "key": data[0],
            "shmid": int(data[1]),
            "owner": data[2],
            "perms": data[3],
            "bytes": int(data[4]),
            "nattch": int(data[5]),
            "status": None,
        }

        if len(data) == 7:
            parsed_data["status"] = data[6]

        return parsed_data

    elif type == SEMAPHORE:
        if len(data) != 5:
            raise ValueError("Unexpected number of outputs for semaphore: " + str(data))

        return {
            "key": data[0],
            "semid": int(data[1]),
            "owner": data[2],
            "perms": data[3],
            "nsems": int(data[4]),
        }
    
    raise ValueError("Unknown type: " + str(type))


def force_stop(t_process):
    """
    Cerca di arrestare forzatamente tutti i processi che hanno il nome <t_process>
    :param str t_process: nome processo da fermare
    :return bool: True se e' riuscito ad eseguire il comando kill per forzare l'arresto dei processi
    """
    if t_process not in ["client_0", "server"]:
        raise ValueError("can't stop" + str(t_process))

    pgrep = subprocess.run(["pgrep", t_process], capture_output=True)
    pids = pgrep.stdout.decode().strip()

    if pids == "":
        return False

    v_pids = pids.splitlines()
    if len(v_pids) > 1:
        print("WARNING: found more than one {} process".format(t_process))
    for pid in v_pids:
        assert subprocess.call(["kill", "-s", "SIGKILL", pid]) == 0, "couldn't kill process " + str(t_process)

    return True


def stress_test(production):
    """
    Verifica che il sistema client + server sia in grado di reggere molte richieste di invio mediante SIGINT.

    Dovrebbe durare circa 7 minuti.

    :param bool production: True per analizzare gli eseguibili senza DEBUG_PRINT
    :return bool: sempre True. Se non ritorna vuol dire che e' avvenuto un errore.
    """
    f_client_log = open("stress_client.log", "w")
    f_server_log = open("stress_server.log", "w")

    try:
        print("[TEST_END2END:STRESS_TEST] COMPILO ESEGUIBILI")
        if production:
            compile_final()
        else:
            compile_debug()

        print("[TEST_END2END:STRESS_TEST] COPIO ESEGUIBILI NELL'AREA DI TEST")
        copy_to_test_area()
        
        print("[TEST_END2END:STRESS_TEST] RIMUOVO EVENTUALI IPC")
        clear_ipcs()
        ipcs = get_ipcs()
        print("[TEST_END2END:STRESS_TEST] START IPCS: ", ipcs)
        for key in ipcs.keys():
            assert len(ipcs[key]) == 0, key + " number should be 0 at the start of the tests"

        print("[TEST_END2END:STRESS_TEST] FACCIO PARTIRE CLIENT E SERVER")
        server_process = subprocess.Popen([server_exe_test_path], stderr=f_server_log)
        client_process = subprocess.Popen([client_0_exe_test_path, test_area_dir], stderr=f_client_log)

        time.sleep(5)

        print("[TEST_END2END:STRESS_TEST] FACCIO PARTIRE I TEST")
        for _ in range(10000):
            # lavora per dare un minimo di tempo all'eseguibile
            files = find_sendme_files()
            expected_outputs = prepare_expected_outputs(files)
            client_process.send_signal(signal.SIGINT)
        
        print("[TEST_END2END:STRESS_TEST] MANDO SEGNALE DI FINE")
        
        client_process.send_signal(signal.SIGUSR1)
        while client_process.poll() == None:
            pass

        print("[TEST_END2END:STRESS_TEST] IPCS DOPO FINE CLIENT: ", get_ipcs())
        server_process.send_signal(signal.SIGINT)
        while server_process.poll() == None:
            pass

        ipcs = get_ipcs()
        print("[TEST_END2END:STRESS_TEST] END IPCS: ", ipcs)
        
        for key in ipcs.keys():
            assert len(ipcs[key]) == 0, key + " number should be 0 at the end of the tests"
        
        assert os.path.isfile("/tmp/fifo1_file.txt") == False, "FIFO 1 file is still there!"
        assert os.path.isfile("/tmp/fifo2_file.txt") == False, "FIFO 2 file is still there!"

    except Exception as e:
        
        while force_stop("client_0"):
            pass

        while force_stop("server"):
            pass
        
        clear_ipcs()
        raise e

    f_client_log.close()
    f_server_log.close()

    print("[TEST_END2END:STRESS_TEST] HO FINITO")
    return True


def soundness_test(production):
    """
    Verifica che l'output del sistema client + server sia corretto.

    Questo test dovrebbe durare circa un quarto d'ora.
    Dura piu' dello stress test perche' attende piu' tempo e verifica
    che i figli abbiano iniziato e poi finito di lavorare sui file

    :param bool production: True per analizzare gli eseguibili senza DEBUG_PRINT
    :return bool: True se tutti i file in output dal sistema contengono l'output atteso
    """
    f_client_log = open("soundness_client.log", "w")
    f_server_log = open("soundness_server.log", "w")

    success = True

    try:
        print("[TEST_END2END:SOUNDNESS_TEST] COMPILO ESEGUIBILI")
        if production:
            compile_final()
        else:
            compile_debug()

        print("[TEST_END2END:SOUNDNESS_TEST] COPIO ESEGUIBILI NELL'AREA DI TEST")
        copy_to_test_area()
        
        print("[TEST_END2END:SOUNDNESS_TEST] RIMUOVO EVENTUALI IPC")
        clear_ipcs()
        ipcs = get_ipcs()
        print("[TEST_END2END:SOUNDNESS_TEST] START IPCS: ", ipcs)
        for key in ipcs.keys():
            assert len(ipcs[key]) == 0, key + " number should be 0 at the start of the tests"

        print("[TEST_END2END:SOUNDNESS_TEST] FACCIO PARTIRE CLIENT E SERVER")
        server_process = psutil.Popen([server_exe_test_path], stderr=f_server_log)
        client_process = psutil.Popen([client_0_exe_test_path, test_area_dir], stderr=f_client_log)

        time.sleep(5)

        print("[TEST_END2END:SOUNDNESS_TEST] FACCIO PARTIRE I TEST")
        n_tests = 200
        try:
            if os.environ["USER"] == "runner": # gh actions
                n_tests = 20
        except KeyError:
            pass

        for _ in range(n_tests):
            files = find_sendme_files()
            expected_outputs = prepare_expected_outputs(files)
            time.sleep(0.5)
            
            sent_signal = False
            while len(client_process.children()) == 0:
                if not sent_signal:
                    client_process.send_signal(signal.SIGINT)
                    sent_signal = True
            
            time.sleep(0.5)
            while len(client_process.children()) > 0:
                pass
            
            for expout_data in expected_outputs:
                print("[TEST_END2END:SOUNDNESS_TEST] VERIFICO CORRETTEZZA OUTPUT OTTENUTO DAL FILE '{}'".format(expout_data["output_file"]))
                with open(expout_data["output_file"], "r") as f_out:
                    real_output = f_out.read()
                    real_output = real_output.strip()

                    regex = r"spedita dal processo (.*) tramite"
                    subst = "spedita dal processo # tramite"
                    real_output_generic = re.sub(regex, subst, real_output, 0, re.MULTILINE)
                    if real_output_generic:
                        real_output = real_output_generic
                    
                    expected_output = expout_data["output"].strip()
                    expected_output_generic = re.sub(regex, subst, expected_output, 0, re.MULTILINE)
                    if expected_output_generic:
                        expected_output = expected_output_generic

                    if real_output != expected_output:
                        print(expout_data["output_file"], "non contiene dati attesi")
                        success = False
                    # print("-"*50)
            
            # print("="*70)
        
        print("[TEST_END2END:SOUNDNESS_TEST] MANDO SEGNALE DI FINE")
        
        client_process.send_signal(signal.SIGUSR1)
        while client_process.poll() == None:
            pass

        print("[TEST_END2END:SOUNDNESS_TEST] IPCS DOPO FINE CLIENT: ", get_ipcs())
        server_process.send_signal(signal.SIGINT)
        while server_process.poll() == None:
            pass

        ipcs = get_ipcs()
        print("[TEST_END2END:SOUNDNESS_TEST] END IPCS: ", ipcs)
        
        for key in ipcs.keys():
            assert len(ipcs[key]) == 0, key + " number should be 0 at the end of the tests"
        
        assert os.path.isfile("/tmp/fifo1_file.txt") == False, "FIFO 1 file is still there!"
        assert os.path.isfile("/tmp/fifo2_file.txt") == False, "FIFO 2 file is still there!"

    except Exception as e:
        success = False
        while force_stop("client_0"):
            pass

        while force_stop("server"):
            pass
        
        clear_ipcs()
        raise e

    f_client_log.close()
    f_server_log.close()

    print("[TEST_END2END:SOUNDNESS_TEST] HO FINITO")
    return success


if __name__ == "__main__":

    args = sys.argv

    if len(args) != 2:
        print("Passare uno dei seguenti parametri: '--stress-prod', '--stress-debug', '--verify-prod', '--verify-debug'")
        exit(1)

    param = args[1]

    if param not in ['--stress-prod', '--stress-debug', '--verify-prod', '--verify-debug']:
        print("parametro invalido, scegliere un parametro tra: '--stress-prod', '--stress-debug', '--verify-prod', '--verify-debug'")
        exit(1)
    
    if param == '--stress-prod':
        print("[TEST_END2END] INIZIA STRESS TEST PRODUZIONE")
        stress_test(production=True)
    elif param == '--stress-debug':
        print("[TEST_END2END] INIZIA STRESS TEST DEBUG")
        stress_test(production=False)
    elif param == '--verify-prod':
        success = soundness_test(production=True)
        if not success:
            print("FAIL SOUNDNESS")
            exit(1)
    else:
        success = soundness_test(production=False)
        if not success:
            print("FAIL SOUNDNESS")
            exit(1)

import paramiko
from subprocess import Popen, PIPE, STDOUT
import re


class ShellHandler:

    def __init__(self, host, user, psw):
        self.ssh = paramiko.SSHClient()
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.ssh.connect(host, username=user, password=psw, port=22)

        channel = self.ssh.invoke_shell()
        self.stdin = channel.makefile('wb')
        self.stdout = channel.makefile('r')

    def __del__(self):
        self.ssh.close()

    def execute(self, cmd):
        """

        :param cmd: the command to be executed on the remote computer
        :examples:  execute('ls')
                    execute('finger')
                    execute('cd folder_name')
        """
        cmd = cmd.strip('\n')
        self.stdin.write(cmd + '\n')
        finish = 'end of stdOUT buffer. finished with exit status'
        echo_cmd = 'echo {} $?'.format(finish)
        self.stdin.write(echo_cmd + '\n')
        shin = self.stdin
        self.stdin.flush()

        shout = []
        sherr = []
        exit_status = 0
        for line in self.stdout:
            if str(line).startswith(cmd) or str(line).startswith(echo_cmd):
                # up for now filled with shell junk from stdin
                shout = []
            elif str(line).startswith(finish):
                # our finish command ends with the exit status
                exit_status = int(str(line).rsplit(maxsplit=1)[1])
                if exit_status:
                    # stderr is combined with stdout.
                    # thus, swap sherr with shout in a case of failure.
                    sherr = shout
                    shout = []
                break
            else:
                # get rid of 'coloring and formatting' special characters
                shout.append(re.compile(r'(\x9B|\x1B\[)[0-?]*[ -/]*[@-~]').sub('', line).
                             replace('\b', '').replace('\r', ''))

        # first and last lines of shout/sherr contain a prompt
        if shout and echo_cmd in shout[-1]:
            shout.pop()
        if shout and cmd in shout[0]:
            shout.pop(0)
        if sherr and echo_cmd in sherr[-1]:
            sherr.pop()
        if sherr and cmd in sherr[0]:
            sherr.pop(0)

        return shin, shout, sherr


ssh_username = "root"  # Must have root privilege to load module
ssh_password = "temppass"
ssh_ip = "192.168.122.10"
module_name = "slime"

client = ShellHandler(ssh_ip, ssh_username, ssh_password)
p = Popen(['bash', '-c',
           "rsync -a --delete --exclude 'cmake-build*' --exclude 'remote/' ./ root@192.168.122.10:/" + ssh_username + "/" + module_name],
          stdout=PIPE, stdin=PIPE, stderr=PIPE)
output = p.communicate()[0]
stdin, stdout, stderr = client.execute("lsmod | grep " + module_name)

if len(stdout) != 0:
    client.execute("rmmod " + module_name)

client.execute("cd /root/slime")
stdin, stdout, stderr = client.execute("pwd")
print(f'STDOUT: {stdout}')
print(f'STDERR: {stderr}')
stdin, stdout, stderr = client.execute("cmake -DCMAKE_BUILD_TYPE=Debug . && make module")
print(f'STDOUT: {stdout}')
print(f'STDERR: {stderr}')
p = Popen(['bash', '-c',
           "rsync -a --delete root@192.168.122.10:/" + ssh_username + "/" + module_name + "/ ./remote"],
          stdout=PIPE, stdin=PIPE, stderr=PIPE)
output = p.communicate()[0]
client.execute("cd src/kernel/")
stdin, stdout, stderr = client.execute("insmod " + module_name + ".ko")
print(f'STDOUT: {stdout}')
print(f'STDERR: {stderr}')
stdin, stdout, stderr = client.execute("lsmod | grep " + module_name)
if len(stdout) == 0:
    exit(-1)

client.execute("cd /sys/module/" + module_name + "/sections")
stdin, stdout, stderr = client.execute("cat .text")
if len(stderr) != 0:
    exit(-1)
text = stdout[0].strip('\n')
text = re.match("^(0[xX])?[A-Fa-f0-9]+$", text).string

stdin, stdout, stderr = client.execute("cat .data")
if len(stderr) != 0:
    exit(-1)
data = stdout[0].strip('\n')
data = re.match("^(0[xX])?[A-Fa-f0-9]+$", data).string

stdin, stdout, stderr = client.execute("cat .bss")
if len(stderr) != 0:
    exit(-1)
bss = stdout[0].strip('\n')
bss = re.match("^(0[xX])?[A-Fa-f0-9]+$", bss).string

gdb_add_sb_file = "add-symbol-file remote/src/kernel/slime.ko " + text + " -s .data " + data + " -s .bss " + bss

f = open(".gdbinit", "w")
f.write(gdb_add_sb_file)
f.close()

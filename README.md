# login-credential-thief
LCT Malware has ability to dump target's browsers username/passwords and send to your web server.

<h3> !!!This project has'nt been completed yet!!! </h3>

# Description
To test the project you need to build in Debug mode ( which is a default mode ), then specify a Hostname ( just for avoiding cmake to fail )
then you can safely execute program in terminal and see your passwords getting dumped into terminal.

<b> Your passwords are not going to be sent anywhere ( even if you specify hostname ) in the prototype </b>

# Building
Command: ```cmake -Bbuild . -DHOSTNAME=127.0.0.1 -DBUILDTYPE=Debug``` <br/>
<i>Sepcifying BUILDTYPE option is optional as Debug Mode is the default</i><br/>
Then you can build with your builder ( Project only tested with Ninja builder )
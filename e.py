import subprocess
subprocess.call(["g++", "/home/natasha/CLionProjects/threads_project/example.cpp","-o", "example" ,"-std=c++11", "-pthread"])
number = int(input("How many times do you want to run file: "))
while number > 0: 
	print("number")
	subprocess.call("./example")
	number -= 1
	

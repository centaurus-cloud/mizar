import inquirer
import netifaces
import os
questions = [
  inquirer.List('Network Interface',
                message="Select Interfaces ?",
                choices= netifaces.interfaces(),
            ),
]
answers = inquirer.prompt(questions)
val_list = list(answers.values()) 
interface_name = str(val_list[0])
print(interface_name)
lines=[]
with open(os.path.join(os.getenv("HOME"),".profile"), "r") as f:
  lines=f.readlines()
lines.append('export INET="%s"' %interface_name)
with open(os.path.join(os.getenv("HOME"),".profile"), 'w') as f:
  for item in lines:
    f.write("%s\n" % item)

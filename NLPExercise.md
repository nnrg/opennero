# Extending NLP Planner #

In this exercise, we will expand the simple parser provided with OpenNERO to handle more complex commands. The parser will use semantic grammar (See Figure 15.10, 15.11). Semantic grammar is a rule based system that allows the flexibility to combine the syntactic and semantic concepts to parse natural language sentences into executable actions. We will be solving the TowerofHanoi mod through natural language inputs from the user. The idea is to convert simple English language sentences into actions which can be executed by OpenNERO.

## Homework skeleton files ##

Start with downloading the [exercise skeleton files](http://opennero.googlecode.com/svn/wiki/Hw4.tar.gz). Copy the folder named Hw4 from the archive to your OpenNERO installation folder. At this point, a new mod named Hw4 should show up in OpenNERO whenever you run it.

The MyDialog class in my\_text\_interface.py handles the input and parsing. This is the file you should be editing, starting with the semantic\_parser and get\_noun\_phrases methods and adding any additional ones that you need.

## Text input interface ##

Once you have downloaded the homework files, run OpenNERO and start the Extended Semantic Parser from Hw4 mod. A pop-up dialog box will open up which will take text input. For a start, enter the following commands in the dialog box and see how the agent responds.

Move Disk1 from Pole1 to pole3
Pick up Disk1 lying on Pole1
Put Disk1 on Pole2

The current parser can only handle three possible types of English language sentences and the actions corresponding to each of them.

| **English Sentence** | **OpenNERO Command** |
|:---------------------|:---------------------|
| Move Disk1 from Pole1 to Pole2 | Mov Disk1 Pole1 Pole2 |
| Pick up Disk1 from Pole1 | Pick Disk1 Pole1 |
| Put down Disk1 on Pole2 | Put Disk1 Pole2 |

Similarly, variants of the above sentences/commands can be made by using Disk1/Disk2/Disk3 and Pole1/Pole2/Pole3. The command format is always

```
S -> VP1 NP1 NP3 NP4 | VP2 NP1 NP3 | VP3 NP1 NP4
VP1 -> move 
VP2 -> pick up 
VP3 -> put
NP1 -> disk1 | disk2 | disk3
NP2 -> pole1 | pole2 | pole3
NP3 -> from NP2
NP4 -> to NP2 | on NP2
```

The current parser will currently accept many commands that do not follow the above grammar, as long as they still begin with VP1, VP2 or VP3. The simulator, however, will only execute legal commands. For example, if Disk1 is lying on Pole1, then you cannot pick it from Pole2 or, if Disk1 is lying on Disk2, then you cannot move Disk2. Remember, the starting configuration of 3-disk planner includes Disk3 lying on Pole1, Disk2 lying on Disk3 and Disk1 lying on Disk2.

## Problem Statement ##

Your assignment is to improve the parser to handle more complex grammar. Specifically, your parser should be able to handle any combination of the following:

```
S -> S and S
S -> VP1 NP1 NP3 NP4 | VP1 NP1 NP4 NP3 | VP2 NP1 NP3 | VP3 NP1 NP4
VP1 -> move 
VP2 -> pick up 
VP3 -> put
NP1 -> disk1 | disk2 | disk3 | it
NP2 -> pole1 | pole2 | pole3 | ADJ pole
NP3 -> from NP2
NP4 -> to NP2 | on NP2
ADJ  -> left | middle | right 
```

Note that there are changes in the grammar which represent the following additional capabilities:

Compound commands: ([Phrase](Verb.md) [Phrases](Noun.md) and [Phrase](Verb.md) [Phrases](Noun.md))
The noun phrases should be able to be in any order that still makes a proper english sentence.
Recognize adjectives to describing the poles, in addition to the proper names ("Pole1", "Pole2" and "Pole3): ("left", "middle" and "right")
Implicit objects: Refer to the last referenced disk as "it"
The following are some examples of these types of sentences:

"Pick up Disk1 from Pole1 and Put Disk1 on Pole3" (compound sentences)
"Move Disk1 from Left Pole to Middle Pole" (adjectives)
"Move Disk1 to Pole3 from Pole1" (reordering)
"Pick up Disk1 from Pole1 and Put it on Pole3" (implicit object)

Your parser should take the English language input from the user dialog box, parse it and convert it into OpenNERO commands mentioned in the table below. The parser should be on the lines of the Semantic Parser, but you are free to define the rules of your own choice.

| **OpenNERO command** |
|:---------------------|
| Mov Disk1 Pole1 Pole2 |
| Pick Disk1 Pole1 |
| Put Disk1 Pole2 |

Some commands will still be beyond your parser's capabilities, and some sommands are simply incomplete. This is fine, however you should recognize that your parser has failed and output an appropriate error. The following are examples of commands that should be rejected by your parser (hint: this can easily be a byproduct of not finding a correct command, there is no need to do seperate, detailed error checking):

"Move Disk1 from Pole1" (No destination for move)
"Move Disk1 to Pole2" (No source)
"Pick up from Pole2" (No disk specified)
"Get Disk1 from Pole1 and move it to the right pole" (Does not begin with a recognized verb phrase)
The MyDialog class in my\_text\_interface.py provides an interface with which to prove feedback about commands. Examples of the use of this interface are provided in the existing parser implementation. Note that there are both "INFO" and "ERROR" messages.

There is no need to check for "impossible" commands, meaning those that are legal commands but cannot be performed in the current state. Such a command will simply not be complied with.

## Debugging ##

If you run into any bugs in your program, you can find the error log file for OpenNERO at one of the following locations:

  * **Linux or Mac:** `~/.opennero/nero_log.txt`
  * **Windows:** `"AppData\Local\OpenNERO\nero_log.txt"` or `"Local Settings\Application Data\OpenNERO\nero_log.txt"` depending on the version of Windows you have.
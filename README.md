# ActiveTask

[![Build status](https://ci.appveyor.com/api/projects/status/github/cemkan/ActiveTask?branch=master&svg=true)](https://ci.appveyor.com/project/cemkan/activetask)

Active Task is a header only library which provides exclusive thread to any class who inherit from it.

ActiveTask is a helper tool that abstracts thread implementation.

#Usage:
1. Inherit from ActiveTask
2. Call ExecuteOnMyThread!

Samples can be found on ActiveTaskTester.

Any function given as parameter to ExecuteOnMyThread will be executed on a task exclusive to the child task. This provides simple thread management:

#Use Cases:
- Create Worker Threads
- Release crucial tasks (like tasks that have strict timing needs) while accessing IO or doing excessive time consuming jobs.

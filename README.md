# ClearDir

This repo contains the Windows application `ClearDir` (previously called
`StartupDelete`) that, when called, clears the directories given as call
parameters.

> [!CAUTION]
> The contents of the given directories are **not** moved to the recycle
> bin but are immediately deleted, so use with caution!

## Syntax
```
ClearDir.exe <Directory> <Directory> ...
```
So a call could look like this:
- `ClearDir.exe C:\TEMP`
- `ClearDir.exe C:\TEMP "D:\Delete me"`
- ...


## Development history

This application was originally developed under the name `StartupDelete`
as part of my [`tools`](https://rle.sh/git-repo/tools) repository.
However, as time moved on I decided to split that repository up into one
repository for each application it contained. And I actively decided
against keeping the file history because...

- all applications had a very short development time with few commits;
- all new repositories would have the commits of all other applications
  too;
- when I decided to split the repository, I also updated the source code
  to match my new default repository structure and to be more efficient;
- functionally, all applications were finished at that time, so I
  wouldn't have to look at previous revisions.

Hence, I decided to do a fresh start for all the tools.

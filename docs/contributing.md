*How to participate in developing the game or editor, using Git.*

### Introduction

> Translations are handled differently, see
> [Localization](localization.md).  

This page describes the procedure to get your code (fixes, patches, new
features, media etc.) included into Stunt Rally.  
*This assumes you have already cloned our code and done something with
it and now are ready to share your stuff.*

If you don't know what to do, but want to help, check out the **new SR3 repo** page [Contributing](https://github.com/stuntrally/stuntrally3/blob/main/docs/Contributing.md),  
[ToDo](http://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php)
with programming tasks with descriptions (sort by priority P, 1 is
critical, higher are less important).

  

### Prerequisites

-   [Git](http://git-scm.com/) version control software
-   [GitHub account](https://github.com/signup/free), it's free (our
    code is hosted there)
-   Preferably some prior Git knowledge.  
    There is a lot of Git tutorials on internet.

### Set up Git

If this is the first time you use Git, setup your name and email:

    $ git config --global user.name "My Name"
    $ git config --global user.email "myemail@example.com"

You also need to generate ssh keys and setup them on GitHub to push
there.  
See how to on
[Linux](https://help.github.com/articles/generating-ssh-keys#platform-linux)
or
[Windows](https://help.github.com/articles/generating-ssh-keys#platform-windows).

  

### Workflow

A simpler way is to make patches, but only for really small things.

Forking is made possible by GitHub and is great for first
contributions.  
In the long run, we probably let you commit directly to our repo, which
saves everyone's time.  
(Preferably in a branch, since CryHam doesn't like surprises, on
master).

1.  [Fork](http://help.github.com/fork-a-repo/) our repository at GitHub
    (done once)
2.  Clone it (done once)
3.  Develop it (fix, modify or add your new stuff)
4.  Test it.
5.  *If you added new files use `git add`*
6.  Commit your changes: `git commit -a -m "Implemented something"` *(or
    "Fixed a crash when..", etc.)*
7.  Push the code to GitHub: `git push`
8.  Create a [pull request](http://help.github.com/send-pull-requests/)
    on GitHub
9.  *Rest.* *(or praise the Lord and pass the ammunition)*
10. Go back to 3.

> Note: it is likely that work still needs to be done before it will be
> accepted.  
> Also, it is possible that it won't be accepted by CryHam at all (if it
> doesn't fit SR).  
> Thus, it's best to discuss it before.

If you work on our repo (without fork) points 1 and 8 are gone.

> *An artificial feedback loop appears between 3-4, and sometimes
> between 5-7 (forgot to add),  
> Point 9 appears randomly between all of others, and imaginary points 0
> and 12 start to appear later.  
> And none of this would happen if we had gone fishing.*

Have fun *(usually between 3 and 4).*

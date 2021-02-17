# Exercise 2

- Compile, and install the `chardev_SDP_lab` driver.

- Compile the program `test_chardev.c`

- After you install the module look at the tail of the file `/var/log/kern.log`, it will suggest you how to create the special file `/dev/chardev_SDP_lab`

- Execute as `root` `test_chardev /dev/chardev_SDP_lab` and verify that it works correctly.

- Give the command `echo something > /dev/chardev_SDP_lab`

- Try `cat /dev/chardev_SDP_lab`

- What is its output?

- Why `test_chardev /dev/chardev_SDP_lab` gives the correct results, whereas `cat` does not behave as expected?

- Correct your device driver so that to cat produces the expected result.

    - In particular, verify that executing in sequence the commands:
      ```
      echo something > /dev/chardev_SDP_lab
      cat /dev/chardev_SDP_lab
      echo xxx > /dev/chardev_SDP_lab
      cat /dev/chardev_SDP_lab
      ```
      you get the strings something and xxx, respectively, nothing else.

    - Verify also that executing in sequence the commands:
      ```
      echo “First and “ > /dev/chardev_SDP_lab
      echo “second string” > /dev/chardev_SDP_lab
      cat /dev/chardev_SDP_lab
      ```
      you get the output First and second string .

#### Hints:
- Use` printk` to follow the control and data flow: this should allow you to figure out what is the difference between `test_chardev` and `cat` and to change your driver code.

Post your code, a description of your solution, and examples of usage of your driver.

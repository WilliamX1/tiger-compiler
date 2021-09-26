#!/usr/bin/env python

name = input("Please enter your name: ")
sid = input("Please enter your student ID: ")

with open('.info', 'w') as f:
    f.write(f"STUDENT_NAME={name}\nSTUDENT_ID={sid}")

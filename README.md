# Contiguous Memory Allocator (COMP 304 Project 2)

This project implements a **contiguous memory allocation simulator** in **C** for  
**COMP 304 – Operating Systems (Fall 2025)**.

The program manages a contiguous memory region and supports **first-fit, best-fit, and worst-fit** allocation strategies, along with memory release, compaction, and status reporting.

---

## Features
- Request memory for a process using:
  - **First Fit (F/f)**
  - **Best Fit (B/b)**
  - **Worst Fit (W/w)**
- Release allocated memory for a process
- Compact fragmented memory into a single hole
- Display allocated and free memory regions with address ranges
- Supports **interactive mode** and **scripted mode**

---


References

//https://docs.u-boot.org/en/latest/develop/printf.htm
//Used this for making sure of my printf syntax
//https://www.geeksforgeeks.org/operating-systems/memory-management-in-operating-system/
//read a little about memory management from here but didnt use much just for context




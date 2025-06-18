# NCU CE3001 Final Project - gem5 + NVMain
## m5out資料夾結構
## Q1 GEM5 + NVMAIN BUILD-UP (40%) 
照著教學做

執行
```
./build/X86/gem5.opt --outdir=m5out/hello configs/example/se.py -c tests/test-progs/hello/bin/x86/linux/hello --cpu-type=TimingSimpleCPU --caches --l2cache --mem-type=NVMainMemory --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config 2>&1 | tee m5out/hello/nvmain.log
```
將原本放於m5out/的檔案整理在m5out/hello/，並記錄terminal輸出資訊(nvmain log)存於nvmain.log
## Q2 Enable L3 last level cache in GEM5 + NVMAIN (15%)
gem5/configs/common/Options.py
```python
    # Cache Options
    parser.add_option("--external-memory-system", type="string",
                      help="use external ports of this port_type for caches")
    parser.add_option("--tlm-memory", type="string",
                      help="use external port for SystemC TLM cosimulation")
    parser.add_option("--caches", action="store_true")
    parser.add_option("--l2cache", action="store_true")
    parser.add_option("--l3cache", action="store_true")
    parser.add_option("--num-dirs", type="int", default=1)
    parser.add_option("--num-l2caches", type="int", default=1)
    parser.add_option("--num-l3caches", type="int", default=1)
    parser.add_option("--l1d_size", type="string", default="64kB")
    parser.add_option("--l1i_size", type="string", default="32kB")
    parser.add_option("--l2_size", type="string", default="2MB")
    parser.add_option("--l3_size", type="string", default="16MB")
    parser.add_option("--l1d_assoc", type="int", default=2)
    parser.add_option("--l1i_assoc", type="int", default=2)
    parser.add_option("--l2_assoc", type="int", default=8)
    parser.add_option("--l3_assoc", type="int", default=16)
    parser.add_option("--cacheline_size", type="int", default=64)
    parser.add_option("--l3_replacement_policy", type="string", default="LRU")
```
gem5/configs/common/Caches.py
```python
class L3Cache(Cache):
    assoc = 4
    tag_latency = 20
    data_latency = 20
    response_latency = 20
    mshrs = 128
    tgts_per_mshr = 64
    write_buffers = 8
```
gem5/configs/common/CacheOptions.py
```python
if options.l2cache:
        # Provide a clock for the L2 and the L1-to-L2 bus here as they
        # are not connected using addTwoLevelCacheHierarchy. Use the
        # same clock as the CPUs.
        system.l2 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                                   size=options.l2_size,
                                   assoc=options.l2_assoc)

        system.tol2bus = L2XBar(clk_domain = system.cpu_clk_domain)

        system.l2.cpu_side = system.tol2bus.master

        if options.l3cache:
            system.l3 = l3_cache_class(clk_domain=system.cpu_clk_domain,
                                       size=options.l3_size,
                                       assoc=options.l3_assoc)
            # replacement policy
            if options.l3_replacement_policy == "LFU":
                system.l3.replacement_policy = LFURP()
            else:
                system.l3.replacement_policy = LRURP()

            # create L3 bus
            system.tol3bus = L2XBar(clk_domain = system.cpu_clk_domain)

            # L2 -> L3 bus
            system.l2.mem_side = system.tol3bus.slave
            
            # L3 -> mem
            system.l3.cpu_side = system.tol3bus.master
            system.l3.mem_side = system.membus.slave

        else:
            system.l2.mem_side = system.membus.slave
```
## Q3 Config last level cache to 2-way and full-way associative cache and test performance (15%)
set

l2cache

cache_size=128kB

l3cache

cache_size=512kB

2-way: l3_assoc=2

full-way: l3_assoc=512*1024/64=8192
![image](https://github.com/user-attachments/assets/9f405f2e-63ab-4e75-a948-ed128e7ca73c)
## Q4 Modify last level cache policy based on frequency based replacement policy (15%)

gem5/configs/common/CacheOptions.py
```python
if options.l2cache:
    ...
    if options.l3cache:
        system.l3 = l3_cache_class(clk_domain=system.cpu_clk_domain,
                                   size=options.l3_size,
                                   assoc=options.l3_assoc)
        # replacement policy
        if options.l3_replacement_policy == "LFU":
            system.l3.replacement_policy = LFURP()
        else:
            system.l3.replacement_policy = LRURP()
    ...
```

![image](https://github.com/user-attachments/assets/e37235f0-068a-46e4-bcdf-501bcbebf168)



## Q5 Test the performance of write back and write through policy based on 4-way associative cache with isscc_pcm(15%) 

gem5/src/common/cache/base.cc
```python
...
    } else if (blk && (pkt->needsWritable() ? blk->isWritable() :
                       blk->isReadable())) {
        // OK to satisfy access
        incHitCount(pkt);
        satisfyRequest(pkt, blk);
        maintainClusivity(pkt->fromCache(), blk);

        // WRITE THROUGH
        // Write back the block if it is writable when we are doing a normal read/write request
        // This has the same effect as write through policy

        if (blk->isWritable()) {
            PacketPtr writeclean_pkt = writecleanBlk(blk, pkt->req->getDest(), pkt->id);
            writebacks.push_back(writeclean_pkt);
        }

        return true;                                                                                                                                                                                                        return true;
    }
...
```

執行

```bash
scons -c build/X86/gem5.opt
scons EXTRAS=../NVmain build/X86/gem5.opt -j 15
```
![image](https://github.com/user-attachments/assets/239364e6-269d-4f5b-8c55-c9999a55ae9b)


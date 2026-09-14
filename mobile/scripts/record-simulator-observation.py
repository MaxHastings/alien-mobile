#!/usr/bin/env python3
"""Record real Simulator frames at observed simulation times, not wall guesses.
Usage: record-simulator-observation.py OUTPUT_DIRECTORY SEED [SPEED]
The selected installed app keeps running after the last capture.
"""
import json, os, pathlib, re, subprocess, sys, time
directory=pathlib.Path(sys.argv[1]);directory.mkdir(parents=True,exist_ok=True)
env=os.environ.copy()
env.update(SIMCTL_CHILD_ALIEN_MOBILE_DEBUG_OVERLAY='1',
           SIMCTL_CHILD_ALIEN_MOBILE_SEED=sys.argv[2],
           SIMCTL_CHILD_ALIEN_MOBILE_OBSERVATION_SPEED=sys.argv[3] if len(sys.argv)>3 else '1')
targets=[0,300,600,1200,1800];captures=[];started=time.monotonic()
with (directory/'simulator.log').open('w') as log:
    process=subprocess.Popen(['xcrun','simctl','launch','--console','booted',
                              'com.example.AlienMobilePrototype'],env=env,stdout=log,stderr=subprocess.STDOUT)
    while targets and process.poll() is None:
        text=(directory/'simulator.log').read_text()
        times=re.findall(r'MATERIAL simulatedSeconds=([0-9.]+)',text)
        simulated=float(times[-1]) if times else 0
        if simulated>=targets[0]:
            target=targets.pop(0);file=directory/f'simulator-{target:04}.png'
            result=subprocess.run(['xcrun','simctl','io','booted','screenshot',str(file)],capture_output=True)
            captures.append(dict(target_seconds=target,observed_simulated_seconds=simulated,
                                 wall_seconds=time.monotonic()-started,file=str(file),exit=result.returncode))
            (directory/'captures.json').write_text(json.dumps(captures,indent=2)+'\n')
            print(captures[-1],flush=True)
        time.sleep(.2)
    if targets:raise SystemExit('App exited before observation completed')
    print('All observation frames captured; app remains running.',flush=True)
    process.wait()

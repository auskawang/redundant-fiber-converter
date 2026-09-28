from pathlib import Path
import subprocess
import argparse, shutil, sys
parser=argparse.ArgumentParser()
parser.add_argument('--gcc',default=shutil.which('arm-none-eabi-gcc'))
args=parser.parse_args()
if not args.gcc: parser.error('Pass --gcc with the full path to arm-none-eabi-gcc')
r=Path(__file__).resolve().parents[1]/'firmware'; a=r/'Appli'
out=r.parent/'build-check'; out.mkdir(exist_ok=True)
gcc=Path(args.gcc)
flags=['-mcpu=cortex-m55','-mthumb','-mfpu=fpv5-d16','-mfloat-abi=hard','-mcmse','-std=gnu11','-DUSE_HAL_DRIVER','-DSTM32N657xx','-ffunction-sections','-fdata-sections','--specs=nano.specs','-Wall','-Wextra']
incs=[a/'include',a/'Core/Inc',r/'Drivers/STM32N6xx_HAL_Driver/Inc',r/'Drivers/CMSIS/Device/ST/STM32N6xx/Include',r/'Drivers/CMSIS/Include']+[x for x in (a/'src').rglob('*') if x.is_dir()]
flags += ['-I'+str(x) for x in incs]
sources=list((a/'src').rglob('*.c'))+list((a/'Core/Src').glob('*.c'))
sources += list((r/'Drivers/STM32N6xx_HAL_Driver/Src').glob('*.c'))
sources+=list((a/'Core/Startup').glob('*.s'))
objects=[]; logs=[]
for i,src in enumerate(sources):
 obj=out/f'{i}.o'; cmd=[str(gcc),*flags,'-c',str(src),'-o',str(obj)]; res=subprocess.run(cmd,capture_output=True,text=True);logs.append(res.stdout+res.stderr)
 if res.returncode: print(src,res.stderr); break
 objects.append(str(obj))
else:
 cmd=[str(gcc),*flags,*objects,'-T'+str(a/'STM32N657L0HXQ_LRUN.ld'),'--specs=nosys.specs','-Wl,--gc-sections','-Wl,-Map='+str(out/'firmware.map'),'-o',str(out/'firmware.elf'),'-lc','-lm'];res=subprocess.run(cmd,capture_output=True,text=True);logs.append(res.stdout+res.stderr);print('Compiled',len(sources),'sources; link exit',res.returncode);print(res.stderr)
(out/'build.log').write_text(''.join(logs));print('warnings:',sum(x.count('warning:') for x in logs))


sys.exit(res.returncode)

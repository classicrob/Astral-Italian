def load_witness(case,text,rows,command):
 if not any(r.get('power_stars')=='242' and r.get('valid')=='1' for r in rows):return False
 if 'FileSelector.Create' in text or 'FileSelector.StartupCreate' in text:return False
 if case=='normal-boot-ui':
  if '--stage' in command:return False
  markers=('FileSelector.Title','FileSelector.FileSelectStart','FileSelector.FileSelect','FileSelector.DemoStartWait')
  at=-1
  for marker in markers:
   at=text.find('[milestone] '+marker,at+1)
   if at<0:return False
  return True
 return '--stage' in command and 'FileSelector.OpenFile' in text

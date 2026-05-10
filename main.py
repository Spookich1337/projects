import os

prefix = input()

lost_disks = {}

for filename in os.listdir('.'):
    if filename.startswith(prefix) and filename.endswith('.csv'):
        server_disks = {}
        
        with open(filename, 'r') as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                disk_type, size = line.split(',')
                size = int(size)
                
                key = (disk_type, size)
                server_disks[key] = server_disks.get(key, 0) + 1
        
        for (disk_type, size), count in server_disks.items():
            if count % 2 != 0:
                lost_disks[disk_type] = lost_disks.get(disk_type, 0) + size

for disk_type in sorted(lost_disks.keys()):
    print(f"{disk_type}:{lost_disks[disk_type]}")

import os
import glob

def main():
    target_dirs = ['src', 'include']
    extensions = ['*.cpp', '*.hpp', '*.h', '*.c']
    
    total_files = 0
    total_lines = 0
    
    # Store counts per extension
    ext_counts = {'.cpp': 0, '.hpp': 0, '.h': 0, '.c': 0}
    ext_lines = {'.cpp': 0, '.hpp': 0, '.h': 0, '.c': 0}

    for d in target_dirs:
        for ext in extensions:
            for filepath in glob.glob(f'{d}/**/{ext}', recursive=True):
                # Ensure we only process files, not directories (though glob with ext usually gets files)
                if os.path.isfile(filepath):
                    file_ext = os.path.splitext(filepath)[1].lower()
                    if file_ext not in ext_counts:
                        ext_counts[file_ext] = 0
                        ext_lines[file_ext] = 0
                        
                    ext_counts[file_ext] += 1
                    total_files += 1
                    
                    try:
                        with open(filepath, 'r', encoding='utf-8') as f:
                            lines = f.readlines()
                            line_count = len(lines)
                            total_lines += line_count
                            ext_lines[file_ext] += line_count
                    except Exception as e:
                        print(f"Could not read {filepath}: {e}")

    print("--- Statistiques du Code Source Druk ---")
    print(f"Fichiers totaux : {total_files}")
    print(f"Lignes totales  : {total_lines:,}".replace(',', ' '))
    print("\n--- Détail par type de fichier ---")
    for ext in sorted(ext_counts.keys()):
        print(f"Fichiers {ext:<5} : {ext_counts[ext]:>4} fichiers, {ext_lines[ext]:>6,} lignes".replace(',', ' '))

if __name__ == '__main__':
    main()

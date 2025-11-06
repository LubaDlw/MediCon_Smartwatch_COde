#!/usr/bin/env python3
"""
Script to add MediCon branding to all watchfaces
Usage: python scripts/add_branding.py
"""

import os
import re
from pathlib import Path

BRANDING_CODE = """    // MediCon Branding
    lv_obj_t *medicon_label_{face_name} = lv_label_create({face_var});
    lv_obj_set_width(medicon_label_{face_name}, LV_SIZE_CONTENT);
    lv_obj_set_height(medicon_label_{face_name}, LV_SIZE_CONTENT);
    lv_obj_set_x(medicon_label_{face_name}, 0);
    lv_obj_set_y(medicon_label_{face_name}, 10);
    lv_obj_set_align(medicon_label_{face_name}, LV_ALIGN_TOP_MID);
    lv_label_set_text(medicon_label_{face_name}, "MediCon");
    lv_obj_set_style_text_font(medicon_label_{face_name}, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(medicon_label_{face_name}, lv_color_hex(0x00A8FF), LV_PART_MAIN | LV_STATE_DEFAULT);
"""

def add_branding_to_file(filepath):
    """Add MediCon branding to a watchface .c file"""
    
    # Read the file
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Extract face name from file path
    face_name = filepath.stem
    face_var = f"face_{face_name}"
    
    # Check if branding already exists
    if "MediCon Branding" in content:
        print(f"✓ {face_name} already has branding")
        return False
    
    # Find the pattern where we add event callback (this is common in all watchfaces)
    pattern = rf'lv_obj_add_event_cb\({face_var}, onFaceEvent, LV_EVENT_ALL, NULL\);'
    
    if not re.search(pattern, content):
        print(f"✗ {face_name} - Pattern not found, skipping")
        return False
    
    # Create branding code for this specific face
    branding = BRANDING_CODE.format(face_name=face_name, face_var=face_var)
    
    # Replace: add branding after the event callback
    new_content = re.sub(
        pattern,
        rf'lv_obj_add_event_cb({face_var}, onFaceEvent, LV_EVENT_ALL, NULL);\n{branding}',
        content
    )
    
    # Write back if changed
    if new_content != content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"✓ {face_name} - Branding added")
        return True
    else:
        print(f"✗ {face_name} - No changes made")
        return False

def main():
    # Get the project root directory
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    faces_dir = project_root / "src" / "faces"
    
    if not faces_dir.exists():
        print(f"Error: Faces directory not found: {faces_dir}")
        return
    
    print("Adding MediCon branding to all watchfaces...")
    print("=" * 60)
    
    # Find all .c files in subdirectories of src/faces
    modified_count = 0
    skipped_count = 0
    
    for c_file in faces_dir.rglob("*.c"):
        # Skip files in the base faces directory (like convert scripts)
        if c_file.parent == faces_dir:
            continue
            
        # Process the file
        if add_branding_to_file(c_file):
            modified_count += 1
        else:
            skipped_count += 1
    
    print("=" * 60)
    print(f"Summary: {modified_count} files modified, {skipped_count} files skipped")

if __name__ == "__main__":
    main()

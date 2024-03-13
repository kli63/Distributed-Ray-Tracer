def read_amc_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
    return lines

def find_last_frame(lines):
    for line in reversed(lines):
        if line.strip().isdigit():
            return int(line.strip())
    return 0

def process_amc_data(lines, start_frame, speed_factor, desired_duration, frame_rate):
    header = []
    frame_data_started = False

    # Identifying and separating the header
    for line in lines:
        if not line.strip().isdigit() and not frame_data_started:
            header.append(line)
        else:
            frame_data_started = True
            break

    # i think this might be right depending on the frame rate, but we can probably default to the original 4346 given by 01_02.amc
    # total_frames = int(desired_duration * frame_rate)
    total_frames = 4346

    processed_data = []
    current_frame = 0
    frame_count = 0
    skip_frame = False
    # Start processing from where header ends
    for line in lines[len(header):]:
        # Check if the line is a frame number
        if line.strip().isdigit():
            frame_number = int(line.strip())
            skip_frame = frame_number < start_frame or frame_count % speed_factor != 0
            if not skip_frame:
                if current_frame < total_frames:
                    # Update frame number
                    processed_data.append(str(current_frame + 1) + '\n')
                    current_frame += 1
                elif current_frame == total_frames:
                    # Stop processing after completing the current frame
                    break
            frame_count += 1
        elif not skip_frame and processed_data:
            # Keep the data for the selected frames
            processed_data.append(line)

    return header, processed_data

def write_amc_file(header, data, output_path):
    with open(output_path, 'w') as file:
        for line in header:
            file.write(line)
        for line in data:
            file.write(line)

# Parameters
input_path = "12_04.amc"
input_duration = float(148.0)
start_time = 60 # in seconds
# start_frame = 6813
speed_factor = 3
desired_duration = 10

# Main
amc_data = read_amc_file(input_path)
last_frame = find_last_frame(amc_data)
frame_rate = last_frame / input_duration
start_frame = int(start_time * frame_rate)

print(start_frame)

header, modified_data = process_amc_data(amc_data, start_frame, speed_factor, desired_duration, frame_rate)
write_amc_file(header, modified_data, "clipped.amc")
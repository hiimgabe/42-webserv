#!/bin/bash

# Define HTTP header for html page 
echo -e "Content-Type: text/html; charset=UTF-8\r\n\r\n"
echo ""

# Get system variables
cpu_usage=$(top -bn1 | grep "Cpu(s)" | awk '{print $2}' | sed 's/%//') # CPU USE
storage_free=$(df / | awk 'NR==2 {print $4}')  # Free space
storage_used=$(df / | awk 'NR==2 {print $3}')  # Used space
disk_usage=$(df -h / | awk 'NR==2 {print $5}' | sed 's/%//')  # Disk usage %
uptime=$(uptime | sed 's/^.*up \([^,]*\), .*$/\1/')  # Uptime

# Storage %
total_storage=$(($storage_free + $storage_used))
storage_used_percentage=$(echo "scale=2; $storage_used / $total_storage * 100" | bc)

# Create Pie 
create_pie_chart() {
    local usage=$1
    local total=100
    local remaining=$(echo "$total - $usage" | bc)

    local angle=$(echo "$usage * 3.6" | bc)
    local large_arc=0
    if (( $(echo "$usage > 50" | bc -l) )); then
        large_arc=1
    fi

    echo "<svg width='200' height='200' viewBox='0 0 32 32'>"
    echo "  <circle r='16' cx='16' cy='16' fill='#eee'></circle>"
    echo "  <path d='M 16,16 L 16,0 A 16,16 0 $large_arc,1 $(echo "16+16*s($angle/57.2958)" | bc -l),$(echo "16-16*c($angle/57.2958)" | bc -l) z' fill='blue'></path>"
    echo "  <text x='16' y='16' font-size='3' text-anchor='middle' dy='.3em' fill='black'>$usage%</text>"
    echo "</svg>"
}

cat <<EOT
<!DOCTYPE html>
<html lang="en">

	<head>
		<meta charset="UTF-8">
		<link rel="stylesheet" href="/assets/css/main.css">
		<link rel="stylesheet" href="/assets/css/header.css">
		<link rel="stylesheet" href="/assets/css/navbar.css">
		<title>CGI test</title>
	</head>

<body>
	<div id="header">
		<a href="/">
			<img id="webserv-logo" src="/assets/img/webserv-logo.png" alt="webserv-logo">
		</a>
	</div>
<div id="container">
    <div>
		<div id="navbar"></div>
		<script>
			fetch('/navbar.html')
			.then(res => res.text())
			.then(data => { document.getElementById('navbar').innerHTML = data; });
		</script>
        <h1 style="text-align: center;">Server Stats</h1>
		<div class="team-profiles">
        <div style="text-align: center;">
            <h2>CPU Usage</h2>
            $(create_pie_chart $cpu_usage)
        </div>
        <div style="text-align: center;">
            <h2>Storage Usage</h2>
            $(create_pie_chart $(printf "%.0f" $storage_used_percentage))
        <h2>Uptime</h2>
        <p>$uptime</p>
        </div>
        <div style="text-align: center;">
            <h2>Disk Usage</h2>
            $(create_pie_chart $disk_usage)
        </div>
    </div>
</div>
</body>

</html>
EOT

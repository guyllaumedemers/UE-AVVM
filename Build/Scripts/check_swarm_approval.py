#!/usr/bin/env python3
import sys
import re
import urllib.request
import json
import subprocess

def get_changelist_description(changelist_num):
    """Fetches the changelist description using the local p4 CLI."""
    try:
        output = subprocess.check_output(['p4', 'describe', '-s', str(changelist_num)], universal_newlines=True)
        return output
    except Exception as e:
        print(f"Trigger Error: Failed to fetch changelist description: {e}")
        return ""

def main():
    # Perforce passes the changelist number as the last argument
    if len(sys.argv) < 2:
        print("Usage: CheckSwarmApproval.py <changelist_number>")
        sys.exit(1)
        
    changelist_num = sys.argv[1]
    desc = get_changelist_description(changelist_num)
    
    # Swarm automatically appends 'Review: <id>' into changelist descriptions
    match = re.search(r'Review:\s*(\d+)', desc)
    if not match:
        print(f"Submit Blocked: Changelist {changelist_num} does not have an associated Swarm Review.")
        print("All changes must be submitted via an approved Perforce Swarm code review workflow.")
        sys.exit(1)
        
    review_id = match.group(1)
    
    # Swarm Configuration (Update with your internal Swarm host and authentication token)
    swarm_url = "https://your-swarm-server.local"
    swarm_token = "YOUR_SWARM_API_TOKEN" # Generated from Swarm user profile
    
    api_url = f"{swarm_url}/api/v9/reviews/{review_id}"
    
    req = urllib.request.Request(api_url)
    req.add_header("Authorization", f"Bearer {swarm_token}")
    
    try:
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode())
            review_data = data.get('review', {})
            
            state = review_data.get('state')
            
            # Check 1: Is the review fully approved?
            if state != 'approved':
                print(f"Submit Blocked: Swarm Review #{review_id} state is '{state}'.")
                print("Requirement: Review state must be 'approved' before submission.")
                sys.exit(1)
                
            # Check 2: Verify CI Test Status (Optional, if tracked via Swarm test integration)
            # Swarm tracks test results; you can inspect review_data fields or test status hooks here.
            
            print(f"Swarm Review #{review_id} validation passed (State: approved). Allowing submit.")
            sys.exit(0)
            
    except urllib.error.HTTPError as e:
        print(f"Submit Blocked: Swarm API returned HTTP error {e.code} when checking Review #{review_id}.")
        sys.exit(1)
    except Exception as e:
        print(f"Submit Blocked: Failed to communicate with Swarm API: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()
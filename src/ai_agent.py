#!/usr/bin/env python3
import sys
import os
import argparse
from openai import OpenAI
from dotenv import load_dotenv 

def main():
    load_dotenv() 

    # 1. Parse arguments passed from the C++ Shell
    parser = argparse.ArgumentParser()
    parser.add_argument("--history", type=str, default="", help="Recent shell history")
    parser.add_argument("--autocomplete", action="store_true", help="Autocomplete mode")
    parser.add_argument("prompt", nargs="+", help="The user's prompt")
    args = parser.parse_args()

    user_prompt = " ".join(args.prompt)
    cwd = os.getcwd()
    
    # 2. Gather Local Context
    try:
        files = os.listdir(cwd)
        file_list = ", ".join(files[:20]) # Limit to 20 files
    except Exception as e:
        file_list = f"Could not read directory: {e}"

    # 3. Construct the System Prompt dynamically based on mode
    if args.autocomplete:
        # Autocomplete Mode: Strict output, no explanations
        system_prompt = (
            "You are a terminal autocomplete engine. "
            "You have access to the user's recent history and current working directory. "
            "The user is currently typing a command and pressed TAB. "
            "Predict the complete command they want to type based on their history and file context. "
            "CRITICAL: Return ONLY the raw predicted command. Do NOT wrap it in tags, backticks, or quotes. No explanations."
        )
        context_message = (
            f"=== CURRENT DIRECTORY ===\n{cwd}\n"
            f"=== FILES ===\n{file_list}\n"
            f"=== HISTORY ===\n{args.history}\n"
            f"=== CURRENTLY TYPING ===\n{user_prompt}"
        )
    else:
        # Standard Mode (The "ai why this failed" / "ai fix" prompt)
        system_prompt = (
            "You are an AI coding assistant embedded directly inside a custom C++ shell. "
            "You have access to the user's current directory, files, and their recent command history. "
            "Look for typos, missing arguments, or file path errors in the history. "
            "You must respond in exactly two parts:\n"
            "1. A very concise explanation of what went wrong.\n"
            "2. The exact corrected command wrapped strictly in <EXEC> and </EXEC> tags.\n\n"
            "Example:\n"
            "You typed `lj` instead of `ls`.\n"
            "<EXEC>ls -la</EXEC>\n\n"
            "CRITICAL: Do not put markdown backticks inside the <EXEC> tags. Only the raw executable command."
        )
        context_message = (
            f"=== SYSTEM CONTEXT ===\n"
            f"Current Directory: {cwd}\n"
            f"Files here: {file_list}\n\n"
            f"=== RECENT HISTORY ===\n"
            f"{args.history}\n"
            f"=== USER REQUEST ===\n"
            f"{user_prompt}"
        )

    # 4. Call the LLM
    try:
        client = OpenAI() 
        response = client.chat.completions.create(
            model="gpt-4o-mini", 
            messages=[
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": context_message}
            ],
            temperature=0.2 # Low temperature for more logical/analytical answers
        )
        
        # 5. Print out the raw response (C++ shell handles formatting/emojis)
        print(response.choices[0].message.content.strip())
        
    except Exception as e:
        # Pass the error string back to C++
        print(f"AI Error: {e}")

if __name__ == "__main__":
    main()
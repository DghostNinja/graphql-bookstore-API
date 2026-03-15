#include "html_generator.h"
#include <string>

using namespace std;

string generatePlaygroundHTML() {
    return "<!DOCTYPE html>\n"
           "<html>\n"
           "<head>\n"
           "    <meta charset=\"utf-8\"/>\n"
            "    <title>GraphQL Playground - GraphQL Bookstore API</title>\n"
           "    <style>\n"
           "        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: #1a1a1a; color: #f0f0f0; }\n"
           "        .warning { background: #fff3cd; border: 1px solid #ffc107; padding: 10px; margin: 20px 0; border-radius: 4px; color: #333; }\n"
           "        .warning h3 { color: #856404; margin-top: 0; }\n"
           "        .code { background: #2d2d2d; padding: 15px; border-radius: 4px; font-family: monospace; color: #f8f8f2; overflow-x: auto; }\n"
           "        .endpoint { background: #e7f3cf; padding: 10px; margin: 5px 0; border-radius: 4px; color: #333; }\n"
           "        input, textarea { background: #2d2d2d; color: #f0f0f0; border: 1px solid #444; padding: 8px; border-radius: 4px; width: 100%; box-sizing: border-box; }\n"
           "        button { background: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; }\n"
           "        button:hover { background: #45a049; }\n"
           "        .auth-section { background: #333; padding: 15px; border-radius: 4px; margin: 20px 0; }\n"
           "        .status { margin: 10px 0; padding: 10px; border-radius: 4px; }\n"
           "        .success { background: #d4edda; color: #155724; }\n"
           "        .error { background: #f8d7da; color: #721c24; }\n"
           "    </style>\n"
           "</head>\n"
           "<body>\n"
           "    <h1>Vulnerable GraphQL Bookstore API - Security Learning Environment</h1>\n"
           "    \n"
           "    <div class=\"warning\">\n"
           "        <h3>⚠️ Security Warning</h3>\n"
           "        <p>This is a <strong>deliberately vulnerable</strong> GraphQL API designed for security education.</p>\n"
           "        <p><strong>DO NOT use in production.</strong></p>\n"
           "    </div>\n"
           "    \n"
           "    <div class=\"auth-section\">\n"
           "        <h2>Authentication</h2>\n"
           "        <div style=\"display: grid; grid-template-columns: 1fr 1fr; gap: 20px;\">\n"
           "            <div>\n"
           "                <h3>Register</h3>\n"
           "                <input type=\"text\" id=\"reg-username\" placeholder=\"Username\"><br/><br/>\n"
           "                <input type=\"text\" id=\"reg-firstname\" placeholder=\"First Name\"><br/><br/>\n"
           "                <input type=\"text\" id=\"reg-lastname\" placeholder=\"Last Name\"><br/><br/>\n"
           "                <input type=\"password\" id=\"reg-password\" placeholder=\"Password\"><br/><br/>\n"
           "                <button onclick=\"register()\">Register</button>\n"
           "            </div>\n"
           "            <div>\n"
           "                <h3>Login</h3>\n"
           "                <input type=\"text\" id=\"login-username\" placeholder=\"Username\"><br/><br/>\n"
           "                <input type=\"password\" id=\"login-password\" placeholder=\"Password\"><br/><br/>\n"
           "                <button onclick=\"login()\">Login</button>\n"
           "            </div>\n"
           "        </div>\n"
           "        <div id=\"auth-status\" style=\"margin-top: 20px;\"></div>\n"
           "    </div>\n"
           "    \n"
           "    <h2>GraphQL Query Tester</h2>\n"
           "    <div>\n"
           "        <textarea id=\"query\" rows=\"15\" cols=\"80\" placeholder=\"Enter your GraphQL query here...\">query {\n"
           "  books {\n"
           "    id\n"
           "    title\n"
           "    price\n"
           "    stockQuantity\n"
           "  }\n"
           "}</textarea>\n"
           "        <br/><br/>\n"
           "        <button onclick=\"executeQuery()\">Execute Query</button>\n"
           "    </div>\n"
           "    <br/>\n"
           "    <div id=\"response\" class=\"code\">Response will appear here...</div>\n"
           "    \n"
           "    <script>\n"
           "        function getAuthHeader() {\n"
           "            const token = localStorage.getItem('token');\n"
           "            return token ? 'Bearer ' + token : '';\n"
           "        }\n"
           "        \n"
           "        function showStatus(message, isError = false) {\n"
           "            const status = document.getElementById('auth-status');\n"
           "            status.innerHTML = `<div class=\"status \\${isError ? 'error' : 'success'}\">\\${message}</div>`;\n"
           "            setTimeout(() => status.innerHTML = '', 3000);\n"
           "        }\n"
           "        \n"
           "        function executeQuery() {\n"
           "            const query = document.getElementById('query').value;\n"
           "            const response = document.getElementById('response');\n"
           "            \n"
           "            fetch('/graphql', {\n"
           "                method: 'POST',\n"
           "                headers: {\n"
           "                    'Content-Type': 'application/json',\n"
           "                    'Authorization': getAuthHeader()\n"
           "                },\n"
           "                body: JSON.stringify({ query: query })\n"
           "            })\n"
           "            .then(r => r.text())\n"
           "            .then(data => {\n"
           "                response.textContent = data;\n"
           "            })\n"
           "            .catch(err => {\n"
           "                response.textContent = 'Error: ' + err;\n"
           "            });\n"
           "        }\n"
           "        \n"
           "        function register() {\n"
           "            const username = document.getElementById('reg-username').value;\n"
           "            const firstName = document.getElementById('reg-firstname').value;\n"
           "            const lastName = document.getElementById('reg-lastname').value;\n"
           "            const password = document.getElementById('reg-password').value;\n"
           "            \n"
           "            const query = `mutation { register(username: \"\\${username}\", firstName: \"\\${firstName}\", lastName: \"\\${lastName}\", password: \"\\${password}\") { success message token user { id username role } } }`;\n"
           "            \n"
           "            fetch('/graphql', {\n"
           "                method: 'POST',\n"
           "                headers: { 'Content-Type': 'application/json' },\n"
           "                body: JSON.stringify({ query: query })\n"
           "            })\n"
           "            .then(r => r.json())\n"
           "            .then(data => {\n"
           "                if (data.data.register && data.data.register.success) {\n"
           "                    localStorage.setItem('token', data.data.register.token);\n"
           "                    showStatus('Registration successful!');\n"
           "                } else {\n"
           "                    showStatus('Registration failed: ' + (data.data.register?.message || 'Unknown error'), true);\n"
           "                }\n"
           "            })\n"
           "            .catch(err => showStatus('Error: ' + err, true));\n"
           "        }\n"
           "        \n"
           "        function login() {\n"
           "            const username = document.getElementById('login-username').value;\n"
           "            const password = document.getElementById('login-password').value;\n"
           "            \n"
           "            const query = `mutation { login(username: \"\\${username}\", password: \"\\${password}\") { success message token user { id username role } } }`;\n"
           "            \n"
           "            fetch('/graphql', {\n"
           "                method: 'POST',\n"
           "                headers: { 'Content-Type': 'application/json' },\n"
           "                body: JSON.stringify({ query: query })\n"
           "            })\n"
           "            .then(r => r.json())\n"
           "            .then(data => {\n"
           "                if (data.data.login && data.data.login.success) {\n"
           "                    localStorage.setItem('token', data.data.login.token);\n"
           "                    showStatus('Login successful!');\n"
           "                } else {\n"
           "                    showStatus('Login failed: ' + (data.data.login?.message || 'Unknown error'), true);\n"
           "                }\n"
           "            })\n"
           "            .catch(err => showStatus('Error: ' + err, true));\n"
           "        }\n"
           "    </script>\n"
           "</body>\n"
           "</html>\n";
}

string generateLandingHTML() {
    return R"HTMLEOF(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GraphQL Bookstore API - Documentation</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            min-height: 100vh;
            font-family: 'Segoe UI', system-ui, sans-serif;
            background: radial-gradient(ellipse at top, #1a1a2e 0%, #0d0d0d 50%, #000000 100%);
            color: #fff;
            overflow-x: hidden;
        }
        .nav-menu {
            background: linear-gradient(135deg, rgba(255,255,255,0.08) 0%, rgba(255,255,255,0.04) 100%);
            backdrop-filter: blur(20px) saturate(180%);
            -webkit-backdrop-filter: blur(20px) saturate(180%);
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
            padding: 15px 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            position: sticky;
            top: 0;
            z-index: 1000;
        }
        .nav-brand h2 {
            font-size: 1.3rem;
            font-weight: 700;
            margin: 0;
            background: linear-gradient(135deg, #fff 0%, #c0c0c0 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }
        .nav-links {
            display: flex;
            gap: 5px;
        }
        .nav-link {
            color: rgba(255, 255, 255, 0.7);
            text-decoration: none;
            padding: 10px 20px;
            border-radius: 12px;
            transition: all 0.25s ease;
            font-weight: 500;
            font-size: 0.9rem;
        }
        .nav-link:hover {
            background: rgba(255, 255, 255, 0.1);
            color: #fff;
        }
        .nav-link.active {
            background: linear-gradient(135deg, rgba(74, 222, 128, 0.2) 0%, rgba(34, 197, 94, 0.2) 100%);
            color: #4ade80;
            border: 1px solid rgba(74, 222, 128, 0.3);
        }
        .github-link {
            display: flex;
            align-items: center;
            gap: 5px;
        }
        .github-link:hover {
            background: rgba(255, 255, 255, 0.1);
            color: #fff;
        }
        .page-content {
            animation: fadeIn 0.3s ease-in-out;
        }
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(10px); }
            to { opacity: 1; transform: translateY(0); }
        }
        .hamburger {
            background: none;
            border: none;
            color: #fff;
            font-size: 1.2rem;
            cursor: pointer;
            margin-right: 15px;
            padding: 5px;
            border-radius: 5px;
            transition: background 0.25s ease;
        }
        .hamburger:hover {
            background: rgba(255, 255, 255, 0.1);
        }
        .sidebar {
            position: fixed;
            top: 0;
            left: -320px;
            width: 300px;
            height: 100vh;
            background: linear-gradient(135deg, rgba(26, 26, 46, 0.98) 0%, rgba(13, 13, 13, 0.98) 100%);
            backdrop-filter: blur(20px);
            -webkit-backdrop-filter: blur(20px);
            border-right: 1px solid rgba(255, 255, 255, 0.1);
            z-index: 2000;
            transition: left 0.3s ease;
            overflow-y: auto;
        }
        .sidebar.active {
            left: 0;
            box-shadow: 0 0 50px rgba(0, 0, 0, 0.8);
        }
        .sidebar-overlay {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100vw;
            height: 100vh;
            background: rgba(0, 0, 0, 0.5);
            z-index: 1999;
        }
        .sidebar-overlay.active {
            display: block;
        }
        .sidebar-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 20px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }
        .sidebar-header h3 {
            margin: 0;
            font-size: 1.2rem;
            color: #4ade80;
        }
        .sidebar-close {
            background: none;
            border: none;
            color: #fff;
            font-size: 1.5rem;
            cursor: pointer;
            padding: 5px;
            border-radius: 5px;
            transition: background 0.25s ease;
        }
        .sidebar-close:hover {
            background: rgba(255, 255, 255, 0.1);
        }
        .sidebar-menu {
            padding: 20px 0;
        }
        .sidebar-item {
            display: block;
            color: rgba(255, 255, 255, 0.8);
            text-decoration: none;
            padding: 15px 20px;
            transition: all 0.25s ease;
            border-left: 3px solid transparent;
        }
        .sidebar-item:hover {
            background: rgba(255, 255, 255, 0.05);
            color: #fff;
            border-left-color: #4ade80;
        }
        .sidebar-item.active {
            background: rgba(74, 222, 128, 0.1);
            color: #4ade80;
            border-left-color: #4ade80;
        }
        .sidebar-group {
            margin-bottom: 10px;
        }
        .sidebar-group-title {
            padding: 15px 20px 5px 20px;
            font-size: 0.75rem;
            font-weight: 600;
            color: rgba(255, 255, 255, 0.5);
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        .sidebar-item.sub-item {
            padding-left: 40px;
            font-size: 0.85rem;
        }
        .doc-section {
            display: none;
            animation: fadeIn 0.3s ease-in-out;
        }
        .doc-section.active {
            display: block;
        }
        .doc-section .code-block {
            height: auto;
            overflow: auto;
        }
        .doc-section .code-block .code-scroller {
            animation: none;
            white-space: pre-wrap;
        }
        .copy-button {
            background: linear-gradient(135deg, rgba(96, 165, 250, 0.2) 0%, rgba(59, 130, 246, 0.2) 100%);
            border: 1px solid rgba(96, 165, 250, 0.3);
            color: #60a5fa;
            border-radius: 6px;
            padding: 4px 8px;
            font-size: 0.7rem;
            cursor: pointer;
            margin-left: 8px;
            transition: all 0.25s ease;
        }
        .copy-button:hover {
            background: linear-gradient(135deg, rgba(96, 165, 250, 0.3) 0%, rgba(59, 130, 246, 0.3) 100%);
            border-color: rgba(96, 165, 250, 0.5);
            transform: translateY(-1px);
        }
        .copy-button:active {
            transform: translateY(0);
        }
        .code-block-with-copy {
            position: relative;
        }
        .code-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 8px;
        }
        .code-title {
            color: #4ade80;
            font-weight: 600;
            font-size: 0.9rem;
        }
        .install-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-bottom: 30px;
        }
        .install-card {
            background: linear-gradient(135deg, rgba(255,255,255,0.03) 0%, rgba(255,255,255,0.01) 100%);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 16px;
            padding: 20px;
            transition: all 0.25s ease;
        }
        .install-card:hover {
            background: rgba(255, 255, 255, 0.05);
            border-color: rgba(255, 255, 255, 0.12);
            transform: translateY(-2px);
        }
        .install-title {
            font-size: 1.1rem;
            font-weight: 600;
            margin-bottom: 15px;
            color: #4ade80;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        @media (max-width: 900px) {
            .install-grid { grid-template-columns: 1fr; }
        }
        @media (max-width: 600px) {
            .sidebar { width: 80vw; left: -85vw; }
            .hamburger { display: block; }
        }
        .glow {
            position: fixed;
            border-radius: 50%;
            filter: blur(100px);
            z-index: 0;
            opacity: 0.4;
        }
        .glow-1 { width: 600px; height: 600px; background: #1a1a2e; top: -200px; left: -100px; animation: pulseGlow 8s ease-in-out infinite; }
        .glow-2 { width: 400px; height: 400px; background: #16213e; bottom: -100px; right: -50px; animation: pulseGlow 10s ease-in-out infinite reverse; }
        .glow-3 { width: 300px; height: 300px; background: #0f0f23; top: 50%; left: 50%; animation: pulseGlow 12s ease-in-out infinite; }
        @keyframes pulseGlow {
            0%, 100% { opacity: 0.3; transform: scale(1); }
            50% { opacity: 0.5; transform: scale(1.1); }
        }
        .container {
            position: relative;
            z-index: 1;
            max-width: 1400px;
            margin: 0 auto;
            padding: 40px;
        }
        .header-card {
            background: linear-gradient(135deg, rgba(255,255,255,0.05) 0%, rgba(255,255,255,0.02) 100%);
            backdrop-filter: blur(30px) saturate(180%);
            -webkit-backdrop-filter: blur(30px) saturate(180%);
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 28px;
            padding: 35px 40px;
            margin-bottom: 25px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.1), 0 0 100px rgba(255,255,255,0.02);
        }
        h1 {
            font-size: 2rem;
            font-weight: 700;
            margin-bottom: 8px;
            background: linear-gradient(135deg, #fff 0%, #c0c0c0 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }
        .subtitle {
            font-size: 1rem;
            color: rgba(255, 255, 255, 0.6);
            margin-bottom: 20px;
        }
        .warning-banner {
            background: linear-gradient(135deg, rgba(255, 193, 7, 0.1) 0%, rgba(255, 193, 7, 0.05) 100%);
            border: 1px solid rgba(255, 193, 7, 0.3);
            border-radius: 14px;
            padding: 14px 18px;
            display: flex;
            align-items: center;
            gap: 12px;
        }
        .warning-icon { font-size: 1.1rem; }
        .warning-text { color: #ffd54f; font-size: 0.9rem; line-height: 1.4; }
        .section-title {
            font-size: 1.1rem;
            font-weight: 600;
            margin-bottom: 16px;
            color: rgba(255, 255, 255, 0.9);
            display: flex;
            align-items: center;
            gap: 10px;
        }
        .section-title::before {
            content: '';
            width: 3px;
            height: 16px;
            background: linear-gradient(180deg, #fff 0%, rgba(255,255,255,0.5) 100%);
            border-radius: 2px;
        }
        .glass-panel {
            background: linear-gradient(135deg, rgba(255,255,255,0.04) 0%, rgba(255,255,255,0.01) 100%);
            backdrop-filter: blur(25px) saturate(150%);
            -webkit-backdrop-filter: blur(25px) saturate(150%);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 22px;
            padding: 25px;
            margin-bottom: 20px;
            box-shadow: 0 4px 24px rgba(0,0,0,0.3), inset 0 1px 0 rgba(255,255,255,0.08);
        }
        .code-block {
            background: rgba(0, 0, 0, 0.6);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 12px;
            padding: 18px;
            font-family: 'Fira Code', 'Consolas', monospace;
            font-size: 0.85rem;
            overflow: hidden;
            color: #b0b0b0;
            line-height: 1.6;
            height: 180px;
        }
        .code-scroller {
            animation: scrollCode 15s linear infinite;
            white-space: nowrap;
        }
        @keyframes scrollCode {
            0% { transform: translateY(0); }
            100% { transform: translateY(-50%); }
        }
        .code-block:hover .code-scroller {
            animation-play-state: paused;
        }
        .code-keyword { color: #a78bfa; }
        .code-string { color: #a3e635; }
        .code-comment { color: #52525b; font-style: italic; }
        .code-number { color: #fb923c; }
        .endpoints-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(220px, 1fr));
            gap: 12px;
        }
        .endpoint-card {
            background: linear-gradient(135deg, rgba(255,255,255,0.03) 0%, rgba(255,255,255,0.01) 100%);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 14px;
            padding: 14px;
            cursor: pointer;
            transition: all 0.25s cubic-bezier(0.4, 0, 0.2, 1);
            position: relative;
            overflow: hidden;
        }
        .endpoint-card::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255,255,255,0.08), transparent);
            transition: left 0.4s ease;
        }
        .endpoint-card:hover {
            background: rgba(255, 255, 255, 0.08);
            border-color: rgba(255, 255, 255, 0.2);
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(0,0,0,0.4);
        }
        .endpoint-card:hover::before { left: 100%; }
        .endpoint-header {
            display: flex;
            align-items: center;
            gap: 8px;
            margin-bottom: 8px;
        }
        .endpoint-method {
            padding: 4px 10px;
            border-radius: 5px;
            font-size: 0.7rem;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.3px;
        }
        .method-query { background: rgba(74, 222, 128, 0.15); color: #4ade80; border: 1px solid rgba(74, 222, 128, 0.3); }
        .method-mutation { background: rgba(244, 114, 182, 0.15); color: #f472b6; border: 1px solid rgba(244, 114, 182, 0.3); }
        .auth-badge {
            padding: 3px 8px;
            border-radius: 8px;
            font-size: 0.7rem;
            font-weight: 600;
        }
        .auth-required { background: rgba(251, 191, 36, 0.15); color: #fbbf24; border: 1px solid rgba(251, 191, 36, 0.3); }
        .auth-optional { background: rgba(74, 222, 128, 0.15); color: #4ade80; border: 1px solid rgba(74, 222, 128, 0.3); }
        .endpoint-name {
            font-size: 1rem;
            font-weight: 600;
            color: #e5e5e5;
            margin-bottom: 4px;
        }
        .endpoint-desc {
            font-size: 0.85rem;
            color: rgba(255, 255, 255, 0.45);
            line-height: 1.4;
        }
        .api-link-container {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 10px;
            margin: 20px auto 25px auto;
            padding: 10px 20px;
            background: linear-gradient(135deg, rgba(255,255,255,0.04) 0%, rgba(255,255,255,0.01) 100%);
            backdrop-filter: blur(25px) saturate(150%);
            -webkit-backdrop-filter: blur(25px) saturate(150%);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 14px;
            box-shadow: 0 4px 24px rgba(0,0,0,0.3), inset 0 1px 0 rgba(255,255,255,0.08);
            width: fit-content;
            animation: containerSlideIn 0.6s cubic-bezier(0.16, 1, 0.3, 1) forwards;
            opacity: 0;
            transform: translateY(-15px);
        }
        @keyframes containerSlideIn {
            to { opacity: 1; transform: translateY(0); }
        }
        .glass-icon {
            width: 28px;
            height: 28px;
            background: linear-gradient(135deg, rgba(74, 222, 128, 0.3) 0%, rgba(34, 197, 94, 0.3) 100%);
            border: 1px solid rgba(74, 222, 128, 0.5);
            border-radius: 8px;
            display: flex;
            align-items: center;
            justify-content: center;
            animation: pulse 2s infinite;
            box-shadow: 0 0 15px rgba(74, 222, 128, 0.3);
        }
        @keyframes pulse {
            0% { box-shadow: 0 0 0 0 rgba(74, 222, 128, 0.5); }
            70% { box-shadow: 0 0 0 8px rgba(74, 222, 128, 0); }
            100% { box-shadow: 0 0 0 0 rgba(74, 222, 128, 0); }
        }
        .glass-icon svg {
            width: 16px;
            height: 16px;
            fill: rgba(74, 222, 128, 0.9);
        }
        .api-link-text {
            font-size: 0.9rem;
            color: rgba(255, 255, 255, 0.6);
            font-style: italic;
            font-family: 'Georgia', serif;
        }
        .api-link {
            color: #4ade80;
            font-weight: 600;
            cursor: pointer;
            text-decoration: none;
            transition: all 0.25s ease;
        }
        .api-link:hover {
            color: #22c55e;
            text-shadow: 0 0 10px rgba(74, 222, 128, 0.5);
        }
        .api-link.copied {
            color: #fbbf24;
        }
        .method-info {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 8px;
            margin-bottom: 20px;
            font-size: 0.85rem;
            color: rgba(255, 255, 255, 0.5);
        }
        .method-badge {
            padding: 3px 8px;
            border-radius: 4px;
            font-size: 0.7rem;
            font-weight: 700;
            font-family: 'Courier New', monospace;
        }
        .method-post {
            background: rgba(168, 85, 247, 0.2);
            color: #a855f7;
            border: 1px solid rgba(168, 85, 247, 0.4);
        }
        .method-get {
            background: rgba(59, 130, 246, 0.2);
            color: #3b82f6;
            border: 1px solid rgba(59, 130, 246, 0.4);
        }
        .method-info-text {
            color: rgba(255, 255, 255, 0.4);
        }
        .vuln-slideshow {
            background: linear-gradient(135deg, rgba(255,255,255,0.03) 0%, rgba(255,255,255,0.01) 100%);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 20px;
            padding: 25px;
            margin-bottom: 25px;
            box-shadow: 0 4px 24px rgba(0,0,0,0.3), inset 0 1px 0 rgba(255,255,255,0.08);
        }
        .vuln-card {
            display: none;
            text-align: center;
            padding: 15px;
            animation: vulnFadeIn 0.5s ease;
        }
        .vuln-card.active {
            display: block;
        }
        @keyframes vulnFadeIn {
            from { opacity: 0; transform: scale(0.95); }
            to { opacity: 1; transform: scale(1); }
        }
        .vuln-chapter {
            font-size: 1.1rem;
            font-weight: 700;
            color: #f472b6;
            margin-bottom: 12px;
            text-transform: uppercase;
            letter-spacing: 2px;
        }
        .vuln-hint {
            font-size: 0.95rem;
            color: rgba(255, 255, 255, 0.7);
            font-style: italic;
            line-height: 1.7;
            max-width: 800px;
            margin: 0 auto;
        }
        .vuln-nav {
            display: flex;
            justify-content: center;
            gap: 10px;
            margin-top: 20px;
        }
        .vuln-dot {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background: rgba(255, 255, 255, 0.2);
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .vuln-dot.active {
            background: #f472b6;
            box-shadow: 0 0 10px rgba(244, 114, 182, 0.5);
        }
        .coupon-carousel {
            background: linear-gradient(135deg, rgba(74, 222, 128, 0.05) 0%, rgba(34, 197, 94, 0.02) 100%);
            border: 1px solid rgba(74, 222, 128, 0.15);
            border-radius: 16px;
            padding: 20px;
            margin-bottom: 25px;
            box-shadow: 0 4px 24px rgba(0,0,0,0.3), inset 0 1px 0 rgba(255,255,255,0.05);
        }
        .coupon-carousel-title {
            text-align: center;
            font-size: 0.85rem;
            font-weight: 600;
            color: rgba(74, 222, 128, 0.8);
            margin-bottom: 15px;
            text-transform: uppercase;
            letter-spacing: 1.5px;
        }
        .coupon-card {
            display: none;
            text-align: center;
            padding: 10px;
            animation: couponFadeIn 0.6s ease;
        }
        .coupon-card.active {
            display: block;
        }
        @keyframes couponFadeIn {
            from { opacity: 0; transform: translateY(10px); }
            to { opacity: 1; transform: translateY(0); }
        }
        .coupon-code {
            display: inline-block;
            font-size: 1.8rem;
            font-weight: 700;
            color: #4ade80;
            background: rgba(74, 222, 128, 0.1);
            border: 1px solid rgba(74, 222, 128, 0.3);
            border-radius: 8px;
            padding: 8px 24px;
            margin-bottom: 10px;
            letter-spacing: 3px;
            cursor: pointer;
            transition: all 0.25s ease;
        }
        .coupon-code:hover {
            background: rgba(74, 222, 128, 0.2);
            box-shadow: 0 0 20px rgba(74, 222, 128, 0.3);
        }
        .coupon-discount {
            font-size: 1rem;
            color: rgba(255, 255, 255, 0.8);
            margin-bottom: 5px;
        }
        .coupon-desc {
            font-size: 0.85rem;
            color: rgba(255, 255, 255, 0.5);
            font-style: italic;
        }
        .coupon-nav {
            display: flex;
            justify-content: center;
            gap: 8px;
            margin-top: 15px;
        }
        .coupon-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background: rgba(255, 255, 255, 0.15);
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .coupon-dot.active {
            background: #4ade80;
            box-shadow: 0 0 8px rgba(74, 222, 128, 0.5);
        }
        .tools-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-bottom: 20px;
        }
        .auth-grid {
            display: grid;
            grid-template-rows: 1fr 1fr;
            gap: 15px;
        }
        .query-panel {
            background: linear-gradient(135deg, rgba(255,255,255,0.04) 0%, rgba(255,255,255,0.01) 100%);
            backdrop-filter: blur(25px) saturate(150%);
            -webkit-backdrop-filter: blur(25px) saturate(150%);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 18px;
            padding: 20px;
            box-shadow: 0 4px 24px rgba(0,0,0,0.3), inset 0 1px 0 rgba(255,255,255,0.08);
        }
        .auth-panel {
            background: linear-gradient(135deg, rgba(255,255,255,0.04) 0%, rgba(255,255,255,0.01) 100%);
            backdrop-filter: blur(25px) saturate(150%);
            -webkit-backdrop-filter: blur(25px) saturate(150%);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 18px;
            padding: 20px;
            box-shadow: 0 4px 24px rgba(0,0,0,0.3), inset 0 1px 0 rgba(255,255,255,0.08);
        }
        .panel-title {
            font-size: 0.95rem;
            font-weight: 600;
            margin-bottom: 14px;
            color: rgba(255, 255, 255, 0.9);
            display: flex;
            align-items: center;
            gap: 8px;
        }
        .panel-title span { font-size: 1rem; }
        .query-input {
            width: 100%;
            background: rgba(0, 0, 0, 0.5);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 10px;
            padding: 12px;
            color: #d4d4d4;
            font-family: 'Fira Code', 'Consolas', monospace;
            font-size: 0.7rem;
            resize: vertical;
            min-height: 140px;
            margin-bottom: 10px;
            transition: all 0.25s ease;
        }
        .query-input:focus {
            outline: none;
            border-color: rgba(167, 139, 250, 0.5);
            box-shadow: 0 0 15px rgba(167, 139, 250, 0.15);
        }
        .query-input::placeholder { color: rgba(255,255,255,0.3); }
        .auth-row {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
            margin-bottom: 10px;
        }
        .auth-input {
            width: 100%;
            background: rgba(0, 0, 0, 0.4);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 8px;
            padding: 10px 12px;
            color: #fff;
            font-size: 0.8rem;
            transition: all 0.25s ease;
        }
        .auth-input:focus {
            outline: none;
            border-color: rgba(167, 139, 250, 0.5);
        }
        .auth-input::placeholder { color: rgba(255,255,255,0.3); }
        .btn-row {
            display: grid;
            grid-template-columns: 1fr 1fr 1fr;
            gap: 8px;
        }
        .btn {
            background: linear-gradient(135deg, rgba(167, 139, 250, 0.2) 0%, rgba(139, 92, 246, 0.2) 100%);
            border: 1px solid rgba(167, 139, 250, 0.3);
            color: #fff;
            border-radius: 10px;
            padding: 10px 12px;
            font-size: 0.8rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.25s ease;
        }
        .btn:hover {
            background: linear-gradient(135deg, rgba(167, 139, 250, 0.3) 0%, rgba(139, 92, 246, 0.3) 100%);
            border-color: rgba(167, 139, 250, 0.5);
            transform: translateY(-1px);
        }
        .btn-primary {
            background: linear-gradient(135deg, rgba(74, 222, 128, 0.2) 0%, rgba(34, 197, 94, 0.2) 100%);
            border: 1px solid rgba(74, 222, 128, 0.3);
        }
        .btn-primary:hover {
            background: linear-gradient(135deg, rgba(74, 222, 128, 0.3) 0%, rgba(34, 197, 94, 0.3) 100%);
            border-color: rgba(74, 222, 128, 0.5);
        }
        .btn-secondary {
            background: linear-gradient(135deg, rgba(96, 165, 250, 0.2) 0%, rgba(59, 130, 246, 0.2) 100%);
            border: 1px solid rgba(96, 165, 250, 0.3);
        }
        .btn-secondary:hover {
            background: linear-gradient(135deg, rgba(96, 165, 250, 0.3) 0%, rgba(59, 130, 246, 0.3) 100%);
            border-color: rgba(96, 165, 250, 0.5);
        }
        .status-msg {
            padding: 10px 12px;
            border-radius: 8px;
            margin-bottom: 10px;
            font-size: 0.75rem;
            display: none;
        }
        .status-success { background: rgba(74, 222, 128, 0.15); color: #4ade80; border: 1px solid rgba(74, 222, 128, 0.2); }
        .status-error { background: rgba(239, 68, 68, 0.15); color: #f87171; border: 1px solid rgba(239, 68, 68, 0.2); }
        .status-loading { background: rgba(96, 165, 250, 0.15); color: #60a5fa; border: 1px solid rgba(96, 165, 250, 0.2); }
        .status-info { background: rgba(251, 191, 36, 0.15); color: #fbbf24; border: 1px solid rgba(251, 191, 36, 0.2); }
        .response-container {
            background: rgba(0, 0, 0, 0.5);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 10px;
            margin-top: 12px;
            max-height: 250px;
            overflow: auto;
        }
        .response-area {
            padding: 12px;
            font-family: 'Fira Code', 'Consolas', monospace;
            font-size: 0.8rem;
            color: #a3a3a3;
            white-space: pre-wrap;
            word-break: break-all;
            margin: 0;
        }
        .token-display {
            background: rgba(0, 0, 0, 0.4);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 8px;
            padding: 10px;
            margin-top: 10px;
            font-family: 'Fira Code', 'Consolas', monospace;
            font-size: 0.6rem;
            color: #a3a3a3;
            word-break: break-all;
            display: none;
        }
        .features-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(140px, 1fr));
            gap: 12px;
        }
        .feature-item {
            background: linear-gradient(135deg, rgba(255,255,255,0.02) 0%, rgba(255,255,255,0.01) 100%);
            border: 1px solid rgba(255, 255, 255, 0.06);
            border-radius: 12px;
            padding: 14px;
            text-align: center;
            transition: all 0.25s ease;
        }
        .feature-item:hover {
            background: rgba(255, 255, 255, 0.05);
            border-color: rgba(255, 255, 255, 0.12);
            transform: translateY(-2px);
        }
        .feature-icon {
            font-size: 1.4rem;
            margin-bottom: 6px;
            display: inline-block;
        }
        .feature-title {
            font-size: 0.75rem;
            font-weight: 600;
            color: rgba(255, 255, 255, 0.8);
        }
        footer {
            text-align: center;
            padding: 30px 20px;
            color: rgba(255, 255, 255, 0.3);
            font-size: 0.75rem;
            margin-top: 20px;
            border-top: 1px solid rgba(255, 255, 255, 0.05);
        }
        .footer-content {
            max-width: 800px;
            margin: 0 auto;
        }
        .footer-brand {
            font-size: 0.9rem;
            font-weight: 600;
            color: rgba(255, 255, 255, 0.5);
            margin-bottom: 15px;
        }
        .footer-links {
            display: flex;
            justify-content: center;
            gap: 25px;
            margin-bottom: 15px;
            flex-wrap: wrap;
        }
        .footer-links a {
            color: rgba(255, 255, 255, 0.6);
            text-decoration: none;
            font-size: 0.8rem;
            transition: color 0.2s ease, transform 0.2s ease;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            width: 36px;
            height: 36px;
            border-radius: 50%;
            background: rgba(255, 255, 255, 0.05);
            cursor: pointer;
            position: relative;
        }
        .footer-links a svg {
        }
        .footer-links a:hover {
            color: #fff;
            transform: translateY(-2px);
            background: rgba(74, 222, 128, 0.2);
        }
        .footer-refs {
            font-size: 0.7rem;
            color: rgba(255, 255, 255, 0.25);
            margin-top: 15px;
            padding-top: 15px;
            border-top: 1px solid rgba(255, 255, 255, 0.05);
            line-height: 1.6;
        }
        .footer-version {
            display: inline-block;
            background: rgba(74, 222, 128, 0.1);
            padding: 2px 8px;
            border-radius: 4px;
            font-size: 0.65rem;
            color: rgba(74, 222, 128, 0.8);
            margin-bottom: 10px;
        }
        @media (max-width: 900px) {
            .tools-grid { grid-template-columns: 1fr; }
            .endpoints-grid { grid-template-columns: repeat(auto-fill, minmax(150px, 1fr)); }
        }
        @media (max-width: 600px) {
            .container { padding: 20px; }
            h1 { font-size: 1.8rem; }
            .subtitle { font-size: 0.9rem; }
            .nav-menu { padding: 10px 15px; }
            .btn-row { grid-template-columns: 1fr; }
            .auth-row { grid-template-columns: 1fr; }
        }
        @media (max-width: 480px) {
            .container { padding: 15px; }
            .endpoints-grid { grid-template-columns: 1fr; }
            .features-grid { grid-template-columns: repeat(auto-fill, minmax(120px, 1fr)); }
            .install-grid { grid-template-columns: 1fr; }
            .nav-menu { padding: 10px 10px; }
            .api-link-container { gap: 5px; padding: 8px 15px; }
            .glass-icon { width: 1.8em; height: 1.8em; box-shadow: 0 0 10px rgba(74, 222, 128, 0.3); }
            .glass-icon svg { width: 70%; height: 70%; }
            .api-link-text { font-size: 0.8rem; }
            .api-link { font-size: 0.8rem; }
            .method-info { flex-direction: column; gap: 5px; margin-bottom: 15px; }
            .method-badge { font-size: 0.6rem; padding: 2px 6px; }
            .method-info-text { text-align: center; font-size: 0.75rem; }
        }
        .feedback-form {
            background: linear-gradient(135deg, rgba(255,255,255,0.03) 0%, rgba(255,255,255,0.01) 100%);
            border: 1px solid rgba(255, 255, 255, 0.08);
            border-radius: 16px;
            padding: 25px;
            margin-top: 20px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        .form-group label {
            display: block;
            color: rgba(255, 255, 255, 0.8);
            font-size: 0.9rem;
            margin-bottom: 8px;
            font-weight: 500;
        }
        .form-group input,
        .form-group textarea {
            width: 100%;
            padding: 14px 16px;
            background: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 10px;
            color: #fff;
            font-size: 0.95rem;
            font-family: inherit;
            transition: all 0.25s ease;
            box-sizing: border-box;
        }
        .form-group input:focus,
        .form-group textarea:focus {
            outline: none;
            border-color: #4ade80;
            background: rgba(74, 222, 128, 0.05);
            box-shadow: 0 0 0 3px rgba(74, 222, 128, 0.1);
        }
        .form-group input::placeholder,
        .form-group textarea::placeholder {
            color: rgba(255, 255, 255, 0.3);
        }
        .form-group textarea {
            resize: vertical;
            min-height: 120px;
        }
        .submit-button {
            background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%);
            color: #000;
            border: none;
            padding: 14px 32px;
            border-radius: 10px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.25s ease;
            width: 100%;
        }
        .submit-button:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(74, 222, 128, 0.3);
        }
        .submit-button:active {
            transform: translateY(0);
        }
        .submit-button:disabled {
            background: rgba(255, 255, 255, 0.1);
            color: rgba(255, 255, 255, 0.3);
            cursor: not-allowed;
            transform: none;
            box-shadow: none;
        }
        .feedback-message {
            margin-top: 15px;
            padding: 12px 16px;
            border-radius: 8px;
            font-size: 0.9rem;
            text-align: center;
            display: none;
        }
        .feedback-message.success {
            display: block;
            background: rgba(74, 222, 128, 0.1);
            border: 1px solid rgba(74, 222, 128, 0.3);
            color: #4ade80;
        }
        .feedback-message.error {
            display: block;
            background: rgba(248, 113, 113, 0.1);
            border: 1px solid rgba(248, 113, 113, 0.3);
            color: #f87171;
        }
        .honeypot-field {
            position: absolute;
            left: -9999px;
            opacity: 0;
            pointer-events: none;
        }
        .floating-feedback {
            position: fixed;
            bottom: 30px;
            right: 30px;
            width: 60px;
            height: 60px;
            background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            box-shadow: 0 8px 25px rgba(74, 222, 128, 0.4);
            z-index: 1000;
            transition: all 0.3s ease;
            text-decoration: none;
        }
        .floating-feedback:hover {
            transform: scale(1.1);
            box-shadow: 0 12px 35px rgba(74, 222, 128, 0.5);
        }
        .floating-feedback svg {
            color: #000;
            width: 28px;
            height: 28px;
        }
        @media (max-width: 600px) {
            .floating-feedback {
                bottom: 20px;
                right: 20px;
                width: 50px;
                height: 50px;
            }
            .floating-feedback svg {
                width: 24px;
                height: 24px;
            }
        }
        .doc-nav {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid rgba(255,255,255,0.1);
        }
        .doc-nav-btn {
            display: inline-flex;
            align-items: center;
            gap: 8px;
            padding: 10px 20px;
            background: rgba(255,255,255,0.05);
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 8px;
            color: rgba(255,255,255,0.8);
            font-size: 14px;
            cursor: pointer;
            transition: all 0.2s ease;
            text-decoration: none;
        }
        .doc-nav-btn:hover {
            background: rgba(255,255,255,0.1);
            border-color: rgba(255,255,255,0.2);
            color: #fff;
        }
        .doc-nav-btn.prev { margin-right: auto; }
        .doc-nav-btn.next { margin-left: auto; }
        .doc-nav-btn svg {
            width: 16px;
            height: 16px;
        }
        @media (max-width: 600px) {
            .doc-nav {
                flex-direction: column;
                gap: 10px;
            }
            .doc-nav-btn {
                width: 100%;
                justify-content: center;
            }
            .doc-nav-btn.prev { margin-right: 0; }
            .doc-nav-btn.next { margin-left: 0; }
        }
    </style>
</head>
<body>
    <div class="glow glow-1"></div>
    <div class="glow glow-2"></div>
    <div class="glow glow-3"></div>

    <a href="javascript:void(0)" class="floating-feedback" onclick="showDocsAndFeedback()" title="Leave Feedback">
        <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M21 15a2 2 0 0 1-2 2H7l-4 4V5a2 2 0 0 1 2-2h14a2 2 0 0 1 2 2z"></path>
        </svg>
    </a>

    <div id="sidebarOverlay" class="sidebar-overlay" onclick="toggleSidebar()"></div>

        <nav class="nav-menu">
            <div class="nav-brand">
                <h2>GraphQL Bookstore</h2>
            </div>
            <div class="nav-links">
                <a href="#" class="nav-link active" onclick="showPage('home')">Home</a>
                <a href="#" class="nav-link" onclick="showPage('docs')">Docs</a>
                <a href="https://github.com/DghostNinja/graphql-bookstore-API" target="_blank" class="nav-link github-link">
                    <svg height="20" width="20" viewBox="0 0 16 16" fill="currentColor">
                        <path d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0016 8c0-4.42-3.58-8-8-8z"/>
                    </svg>
                </a>
            </div>
        </nav>

        <div id="sidebar" class="sidebar">
            <div class="sidebar-header">
                <h3>Documentation</h3>
                <button class="sidebar-close" onclick="toggleSidebar()">×</button>
            </div>
            <div class="sidebar-menu">
                <div class="sidebar-group">
                    <div class="sidebar-group-title">Overview</div>
                    <a href="#" class="sidebar-item sub-item" onclick="showSection('overview')">Welcome</a>
                    <a href="#" class="sidebar-item sub-item" onclick="showSection('local-install')">Local Installation</a>
                    <a href="#" class="sidebar-item sub-item" onclick="showSection('docker-install')">Docker Installation</a>
                    <a href="#" class="sidebar-item sub-item" onclick="showSection('how-to-send-requests')">How To Send Requests</a>
                    <a href="#" class="sidebar-item sub-item" onclick="showSection('data-types')">Data Types & Schema</a>
                    <a href="#" class="sidebar-item sub-item" onclick="showSection('queries')">Queries</a>
                    <a href="#" class="sidebar-item sub-item" onclick="showSection('mutations')">Mutations</a>
                    <a href="#" class="sidebar-item sub-item" onclick="showSection('vulnerabilities')">Vulnerabilities</a>
                    <a href="#" class="sidebar-item sub-item" onclick="showSection('mcp-docs')">MCP Server</a>
                    <a href="#" class="sidebar-item sub-item" onclick="showSection('feedback')">Feedback</a>
                </div>
            </div>
        </div>

        <div class="container">
            <div id="homePage" class="page-content">
        <div class="header-card">
            <h1>GraphQL Bookstore API</h1>
            <p class="subtitle">Security Learning Environment - Deliberately Vulnerable GraphQL API</p>
            <div class="warning-banner">
                <span class="warning-icon">&#9888;</span>
                <div class="warning-text">
                    <strong>Security Warning:</strong> This API contains intentional vulnerabilities for educational purposes. 
                    <strong>DO NOT deploy in production!</strong>
                </div>
            </div>
            <div class="api-link-container">
                <div class="glass-icon">
                    <svg viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
                        <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm-1 17.93c-3.95-.49-7-3.85-7-7.93 0-.62.08-1.21.21-1.79L9 15v1c0 1.1.9 2 2 2v1.93zm6.9-2.54c-.26-.81-1-1.39-1.9-1.39h-1v-3c0-.55-.45-1-1-1H8v-2h2c.55 0 1-.45 1-1V7h2c1.1 0 2-.9 2-2v-.41c2.93 1.19 5 4.06 5 7.41 0 2.08-.8 3.97-2.1 5.39z"/>
                    </svg>
                </div>
                <span class="api-link-text">Access the API at:</span>
                <a class="api-link" onclick="copyApiLink()" id="apiLink">api.graphqlbook.store/graphql</a>
            </div>
            
            <div class="method-info">
                <span class="method-badge method-post">POST</span> Queries & Mutations &nbsp;|&nbsp; 
                <span class="method-badge method-get">GET</span> Queries only &nbsp;|&nbsp;
                <span class="method-info-text">Use with online GraphQL tools, Postman, curl, etc.</span>
            </div>
        </div>

        <div class="vuln-slideshow">
            <div class="vuln-card active" data-index="0">
                <div class="vuln-chapter">Chapter I: The Injection</div>
                <div class="vuln-hint">"Beyond the veil of queries lies a passage where shadows speak in database tongues. Those who master the ancient art of string concatenation may bend the data realm to their will, extracting secrets from tables unseen."</div>
            </div>
            <div class="vuln-card" data-index="1">
                <div class="vuln-chapter">Chapter II: The Broken Access</div>
                <div class="vuln-hint">"In the halls of the API, doors stand unlocked for all who seek. The internal chambers of user data lie bare to any traveler - no guardian questions those who walk the hidden corridors."</div>
            </div>
            <div class="vuln-card" data-index="2">
                <div class="vuln-chapter">Chapter III: The Misplaced Trust</div>
                <div class="vuln-hint">"The order bears the mark of its creator, yet any hand may seize it. When ownership goes unverified, the boundaries between yours and theirs dissolve into shadow."</div>
            </div>
            <div class="vuln-card" data-index="3">
                <div class="vuln-chapter">Chapter IV: The Open Ledger</div>
                <div class="vuln-hint">"Fields once forbidden now yield to the clever coder's touch. When all paths lead to the throne, when every attribute accepts the whisperer's will, true power transcends mere mortal constraints."</div>
            </div>
            <div class="vuln-card" data-index="4">
                <div class="vuln-chapter">Chapter V: The Extending Reach</div>
                <div class="vuln-hint">"Beyond the visible web lies a realm of machines and metadata. The curious can traverse the boundary between the public face and the hidden infrastructure that powers the kingdom."</div>
            </div>
            <div class="vuln-card" data-index="5">
                <div class="vuln-chapter">Chapter VI: The Unwatched Treasury</div>
                <div class="vuln-hint">"Crown jewels - order records, payment ledgers, system statistics - all lie within reach of the unguarded gate. The keepers have left their treasures unwatched, accessible to any who know where to look."</div>
            </div>
            <div class="vuln-card" data-index="6">
                <div class="vuln-chapter">Chapter VII: The Skeleton Key</div>
                <div class="vuln-hint">"A single key unlocks many doors, yet this key was forged from common words. The authentication bearer need only speak the secret phrase to command the realm's resources."</div>
            </div>
            <div class="vuln-nav">
                <div class="vuln-dot active" onclick="showVulnCard(0)"></div>
                <div class="vuln-dot" onclick="showVulnCard(1)"></div>
                <div class="vuln-dot" onclick="showVulnCard(2)"></div>
                <div class="vuln-dot" onclick="showVulnCard(3)"></div>
                <div class="vuln-dot" onclick="showVulnCard(4)"></div>
                <div class="vuln-dot" onclick="showVulnCard(5)"></div>
                <div class="vuln-dot" onclick="showVulnCard(6)"></div>
            </div>
        </div>

        <div class="coupon-carousel">
            <div class="coupon-carousel-title">Available Coupon Codes</div>
            <div class="coupon-card active" data-index="0">
                <div class="coupon-code" onclick="copyCoupon('WELCOME10')">WELCOME10</div>
                <div class="coupon-discount">10% Off</div>
                <div class="coupon-desc">Welcome discount for new users</div>
            </div>
            <div class="coupon-card" data-index="1">
                <div class="coupon-code" onclick="copyCoupon('FLAT20')">FLAT20</div>
                <div class="coupon-discount">$20 Off</div>
                <div class="coupon-desc">On orders over $100</div>
            </div>
            <div class="coupon-card" data-index="2">
                <div class="coupon-code" onclick="copyCoupon('SUMMER25')">SUMMER25</div>
                <div class="coupon-discount">25% Off</div>
                <div class="coupon-desc">Summer sale - orders over $50</div>
            </div>
            <div class="coupon-card" data-index="3">
                <div class="coupon-code" onclick="copyCoupon('DISCOUNT10')">DISCOUNT10</div>
                <div class="coupon-discount">10% Off</div>
                <div class="coupon-desc">General discount code</div>
            </div>
            <div class="coupon-nav">
                <div class="coupon-dot active" onclick="showCouponCard(0)"></div>
                <div class="coupon-dot" onclick="showCouponCard(1)"></div>
                <div class="coupon-dot" onclick="showCouponCard(2)"></div>
                <div class="coupon-dot" onclick="showCouponCard(3)"></div>
            </div>
        </div>

        <div class="tools-grid">
            <div class="query-panel">
                <div class="panel-title"><span>&#9654;</span> Query Runner</div>
                <textarea id="queryInput" class="query-input" placeholder="Enter GraphQL query..."></textarea>
                <div class="btn-row">
                    <button class="btn btn-primary" onclick="runQuery()">Run Query</button>
                    <button class="btn btn-secondary" onclick="clearQuery()">Clear</button>
                    <button class="btn" onclick="loadSample()">Load Sample</button>
                </div>
                <div id="queryStatusMsg" class="status-msg"></div>
                <div id="responseContainer" class="response-container">
                    <pre id="responseArea" class="response-area">Response will appear here...</pre>
                </div>
            </div>

            <div class="auth-grid">
                <div class="auth-panel">
                    <div class="panel-title"><span>&#128274;</span> Login</div>
                    <div class="auth-row">
                        <input type="text" id="loginUsername" class="auth-input" placeholder="Username">
                        <input type="password" id="loginPassword" class="auth-input" placeholder="Password">
                    </div>
                    <button class="btn btn-primary" style="width:100%" onclick="doLogin()">Login</button>
                    <div id="loginStatusMsg" class="status-msg"></div>
                    <div id="loginTokenDisplay" class="token-display"></div>
                </div>

                <div class="auth-panel">
                    <div class="panel-title"><span>&#128221;</span> Register</div>
                    <div class="auth-row">
                        <input type="text" id="regUsername" class="auth-input" placeholder="Username">
                        <input type="password" id="regPassword" class="auth-input" placeholder="Password">
                    </div>
                    <div class="auth-row">
                        <input type="text" id="regFirstName" class="auth-input" placeholder="First Name">
                        <input type="text" id="regLastName" class="auth-input" placeholder="Last Name">
                    </div>
                    <button class="btn btn-secondary" style="width:100%" onclick="doRegister()">Register</button>
                    <div id="regStatusMsg" class="status-msg"></div>
                    <div id="regTokenDisplay" class="token-display"></div>
                </div>
            </div>
        </div>

        <div class="glass-panel">
            <div class="section-title">Quick Examples</div>
            <div class="code-block">
                <div class="code-scroller">
                <code><span class="code-comment"># Query books (no auth)</span> <span class="code-keyword">query</span> { books(limit:5) { id title price } }<br>
                <span class="code-comment"># Get book by ID</span> <span class="code-keyword">query</span> { book(id:1) { id title description author { firstName lastName } } }<br>
                <span class="code-comment"># Search books</span> <span class="code-keyword">query</span> { books(search:"code") { id title price } }<br>
                <span class="code-comment"># Login</span> <span class="code-keyword">mutation</span> { login(username:"admin", password:"password123") { success token } }<br>
                <span class="code-comment"># Register</span> <span class="code-keyword">mutation</span> { register(username:"user", firstName:"John", lastName:"Doe", password:"pass") { success } }<br>
                <span class="code-comment"># Get current user</span> <span class="code-keyword">query</span> { me { id username role firstName } }<br>
                <span class="code-comment"># Update profile</span> <span class="code-keyword">mutation</span> { updateProfile(phone:"1234567890", city:"NYC") { success } }<br>
                <span class="code-comment"># Add to cart</span> <span class="code-keyword">mutation</span> { addToCart(bookId:1, quantity:2) { success } }<br>
                <span class="code-comment"># View cart</span> <span class="code-keyword">query</span> { cart { id items { bookId quantity } } }<br>
                <span class="code-comment"># Remove from cart</span> <span class="code-keyword">mutation</span> { removeFromCart(bookId:1) { success } }<br>
                <span class="code-comment"># Create order</span> <span class="code-keyword">mutation</span> { createOrder { success orderId } }<br>
                <span class="code-comment"># View orders</span> <span class="code-keyword">query</span> { orders { id orderNumber status totalAmount } }<br>
                <span class="code-comment"># Cancel order</span> <span class="code-keyword">mutation</span> { cancelOrder(orderId:"xxx") { success } }<br>
                <span class="code-comment"># Create review</span> <span class="code-keyword">mutation</span> { createReview(bookId:1, rating:5, comment:"Great!") { success } }<br>
                <span class="code-comment"># View reviews</span> <span class="code-keyword">query</span> { bookReviews(bookId:1) { id rating comment } }<br>
                <span class="code-comment"># Delete review</span> <span class="code-keyword">mutation</span> { deleteReview(reviewId:1) { success } }<br>
                <span class="code-comment"># Register webhook</span> <span class="code-keyword">mutation</span> { registerWebhook(url:"https://example.com", events:"order.created") { success } }<br>
                <span class="code-comment"># Test webhook</span> <span class="code-keyword">mutation</span> { testWebhook(webhookId:"xxx") { success } }<br>
                <span class="code-comment"># Advanced search</span> <span class="code-keyword">query</span> { _searchAdvanced(query:"python") { id title price } }<br>
                <span class="code-comment"># External resource</span> <span class="code-keyword">query</span> { _fetchExternalResource(url:"http://example.com") { content } }
                </code>
                </div>
            </div>
        </div>

        <div class="glass-panel">
            <div class="section-title">Available Queries</div>
            <div class="endpoints-grid">
                <div class="endpoint-card" onclick="setQuery(1)">
                    <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-optional">No Auth</span></div>
                    <div class="endpoint-name">books</div>
                    <div class="endpoint-desc">List books with filters</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(2)">
                    <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-optional">No Auth</span></div>
                    <div class="endpoint-name">book(id)</div>
                    <div class="endpoint-desc">Get book details</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(3)">
                    <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-required">Auth</span></div>
                    <div class="endpoint-name">me</div>
                    <div class="endpoint-desc">Get current user</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(4)">
                    <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-required">Auth</span></div>
                    <div class="endpoint-name">cart</div>
                    <div class="endpoint-desc">Get shopping cart</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(5)">
                    <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-required">Auth</span></div>
                    <div class="endpoint-name">orders</div>
                    <div class="endpoint-desc">Get order history</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(6)">
                    <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-optional">No Auth</span></div>
                    <div class="endpoint-name">_searchAdvanced</div>
                    <div class="endpoint-desc">Advanced book search</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(7)">
                    <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-optional">No Auth</span></div>
                    <div class="endpoint-name">_internalUserSearch</div>
                    <div class="endpoint-desc">Search users by username</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(16)">
                    <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-optional">No Auth</span></div>
                    <div class="endpoint-name">_fetchExternalResource</div>
                    <div class="endpoint-desc">Fetch external URL content</div>
                </div>
            </div>
        </div>

        <div class="glass-panel">
            <div class="section-title">Available Mutations</div>
            <div class="endpoints-grid">
                <div class="endpoint-card" onclick="setQuery(8)">
                    <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-optional">No Auth</span></div>
                    <div class="endpoint-name">register</div>
                    <div class="endpoint-desc">Create account</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(9)">
                    <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-optional">No Auth</span></div>
                    <div class="endpoint-name">login</div>
                    <div class="endpoint-desc">Get JWT token</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(10)">
                    <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                    <div class="endpoint-name">addToCart</div>
                    <div class="endpoint-desc">Add to cart</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(11)">
                    <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                    <div class="endpoint-name">createOrder</div>
                    <div class="endpoint-desc">Create order</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(12)">
                    <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                    <div class="endpoint-name">cancelOrder</div>
                    <div class="endpoint-desc">Cancel order</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(13)">
                    <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                    <div class="endpoint-name">createReview</div>
                    <div class="endpoint-desc">Add review</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(14)">
                    <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                    <div class="endpoint-name">deleteReview</div>
                    <div class="endpoint-desc">Delete review</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(15)">
                    <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                    <div class="endpoint-name">updateProfile</div>
                    <div class="endpoint-desc">Update user profile</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(17)">
                    <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                    <div class="endpoint-name">registerWebhook</div>
                    <div class="endpoint-desc">Register webhook</div>
                </div>
                <div class="endpoint-card" onclick="setQuery(18)">
                    <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                    <div class="endpoint-name">testWebhook</div>
                    <div class="endpoint-desc">Test webhook endpoint</div>
                </div>
            </div>
        </div>

        <div class="glass-panel">
            <div class="section-title">Features</div>
            <div class="features-grid">
                <div class="feature-item"><div class="feature-icon">&#128218;</div><div class="feature-title">Book Catalog</div></div>
                <div class="feature-item"><div class="feature-icon">&#128722;</div><div class="feature-title">Shopping Cart</div></div>
                <div class="feature-item"><div class="feature-icon">&#128221;</div><div class="feature-title">Reviews</div></div>
                <div class="feature-item"><div class="feature-icon">&#128279;</div><div class="feature-title">Webhooks</div></div>
                <div class="feature-item"><div class="feature-icon">&#128274;</div><div class="feature-title">JWT Auth</div></div>
                <div class="feature-item"><div class="feature-icon">&#129179;</div><div class="feature-title">Orders</div></div>
            </div>
        </div>
            </div>

            <div id="docsPage" class="page-content" style="display: none;">
                <nav class="nav-menu">
                    <div class="nav-brand">
                        <button class="hamburger" onclick="toggleSidebar()">☰</button>
                        <h2>Documentation</h2>
                    </div>
                    <div class="nav-links">
                        <a href="#" class="nav-link" onclick="showPage('home')">Back to Home</a>
                    </div>
                </nav>

                <div class="container" style="margin-top: 20px;">
                    <div class="header-card">
                        <h1>API Documentation</h1>
                        <p class="subtitle">Complete guide to GraphQL Bookstore API</p>
                    </div>

                <!-- Welcome Section -->
                <div id="overview" class="doc-section glass-panel">
                    <div class="section-title">Welcome</div>
                    
                    <p style="color: rgba(255,255,255,0.7); margin-bottom: 20px; line-height: 1.6;">
                        Welcome to the <strong>GraphQL Bookstore API</strong> - a deliberately vulnerable API designed for security education and hands-on learning of GraphQL fundamentals. This project provides a realistic bookstore environment where you can explore common web vulnerabilities in a safe, educational setting.
                    </p>

                    <div class="section-title" style="margin-top: 25px;">What is This API?</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        The GraphQL Bookstore API simulates a real-world e-commerce platform built with GraphQL. It includes complete functionality for a bookstore including user accounts, book browsing, shopping cart, order management, product reviews, and webhook notifications. The API is intentionally built with various security vulnerabilities to help developers, security researchers, and students understand and identify common web application security flaws.
                    </p>

                    <div class="section-title" style="margin-top: 25px;">What is It Built For?</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        This API serves as a learning platform for:
                    </p>
                    <ul style="color: rgba(255,255,255,0.6); margin-left: 20px; margin-bottom: 20px; line-height: 1.8;">
                        <li>Understanding GraphQL API security fundamentals</li>
                        <li>Learning to identify and exploit common vulnerabilities</li>
                        <li>Practicing secure coding techniques</li>
                        <li>Testing security tools and methodologies</li>
                        <li>Educational workshops and capture-the-flag (CTF) events</li>
                    </ul>

                    <div class="section-title" style="margin-top: 25px;">Business Flow</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        The API follows a typical e-commerce workflow:
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;">
1. USER REGISTRATION/LOGIN
   └─→ Create account or authenticate to receive JWT token

2. BROWSE BOOKS
   └─→ Query books, search by title/author/category
   └─→ View book details and reviews

3. MANAGE CART
   └─→ Add books to shopping cart
   └─→ Update quantities or remove items
   └──→ Apply coupon codes (discounts)

4. CHECKOUT & PAYMENT
   └─→ Review cart contents
   └─→ Process payment (simulated Vulnbank)
   └─→ Create order record

5. ORDER MANAGEMENT
   └─→ View order history
   └─→ Cancel orders (if allowed)
   └─→ Track order status

6. REVIEWS & RATINGS
   └─→ Submit reviews for purchased books
   └─→ View reviews from other users

7. WEBHOOKS (Optional)
   └─→ Register webhook URLs for real-time notifications
   └─→ Receive events: order created, paid, cancelled, etc.</pre>
                    </div>

                    <div class="doc-nav">
                        <a href="#" class="doc-nav-btn prev" onclick="return false;" style="visibility: hidden;">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 18l-6-6 6-6"/></svg>
                            Previous
                        </a>
                        <a href="#" class="doc-nav-btn next" onclick="showSection('local-install'); return false;">
                            Next
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18l6-6-6-6"/></svg>
                        </a>
                    </div>
                </div>

                <!-- How To Send Requests Section -->
                <div id="how-to-send-requests" class="doc-section glass-panel">
                    <div class="section-title">How To Send Requests</div>
                    
                    <p style="color: rgba(255,255,255,0.7); margin-bottom: 20px; line-height: 1.6;">
                        The GraphQL Bookstore API accepts requests via HTTP POST to the <code style="color: #4ade80;">/graphql</code> endpoint. All requests must include a JSON body with a <code style="color: #fbbf24;">query</code> field containing your GraphQL operation.
                    </p>

                    <div class="section-title" style="margin-top: 25px;">API Endpoints</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        You can use either of the following endpoints:
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #4ade80;">Local Development:</span>  http://localhost:4000/graphql
<span style="color: #60a5fa;">Live URL:</span> https://api.graphqlbook.store/graphql

<span style="color: #52525b;">// All examples below use localhost. Simply replace with the live URL when ready.</span></pre>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Basic Request Structure</div>
                    <div class="code-block">
                        <pre style="color: #a3a3a3;">POST /graphql
Content-Type: application/json

{
  "query": "Your GraphQL query or mutation here"
}</pre>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Using cURL</div>
                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Query Books (No Auth Required)</div>
                            <button class="copy-button" onclick="copyToClipboard('curl-books')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="curl-books">curl -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query":"{ books { id title price } }"}'</pre>
                        </div>
                    </div>

                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Login (Get JWT Token)</div>
                            <button class="copy-button" onclick="copyToClipboard('curl-login')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="curl-login">curl -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -d '{"query":"mutation { login(username: \"admin\", password: \"password123\") { success token } }"}'</pre>
                        </div>
                    </div>

                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Authenticated Request</div>
                            <button class="copy-button" onclick="copyToClipboard('curl-auth')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="curl-auth">curl -X POST http://localhost:4000/graphql \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_JWT_TOKEN_HERE" \
  -d '{"query":"mutation { addToCart(bookId: 1, quantity: 2) { success } }"}'</pre>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Using JavaScript (Fetch API)</div>
                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Basic Query</div>
                            <button class="copy-button" onclick="copyToClipboard('js-basic')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="js-basic">fetch('/graphql', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    query: '{ books { id title price } }'
  })
})
.then(r => r.json())
.then(d => console.log(d));</pre>
                        </div>
                    </div>

                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">With Authentication</div>
                            <button class="copy-button" onclick="copyToClipboard('js-auth')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="js-auth">// First login to get token, then use it for authenticated requests
const token = 'YOUR_JWT_TOKEN_HERE';

fetch('/graphql', {
  method: 'POST',
  headers: { 
    'Content-Type': 'application/json',
    'Authorization': 'Bearer ' + token
  },
  body: JSON.stringify({
    query: 'mutation { addToCart(bookId: 1, quantity: 2) { success } }'
  })
})
.then(r => r.json())
.then(d => console.log(d));</pre>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Query vs Mutation</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        <strong>Queries</strong> are used to fetch data (read operations). <strong>Mutations</strong> are used to modify data (write operations like creating, updating, or deleting).
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #a78bfa;"># Query - Read data (no exclamation point)</span>
query {
  books { id title price }
}

<span style="color: #f472b6;"># Mutation - Write data (starts with mutation keyword)</span>
mutation {
  login(username: "admin", password: "pass") { token }
}</pre>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Request Headers</div>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #fbbf24;">Content-Type:</span> application/json
<span style="color: #fbbf24;">Authorization:</span> Bearer &lt;your-jwt-token&gt;  <span style="color: #52525b;">// Only for authenticated requests</span></pre>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Response Format</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        Successful responses are wrapped in a <code style="color: #fbbf24;">data</code> object. Errors are returned in an <code style="color: #f87171;">errors</code> array.
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #52525b;">// Success Response</span>
{
  "data": {
    "books": [
      { "id": "1", "title": "Book Title", "price": 19.99 }
    ]
  }
}

<span style="color: #52525b;">// Error Response</span>
{
  "errors": [
    { "message": "Authentication required" }
  ]
}</pre>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Using Postman</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        Postman is a popular tool for testing APIs. Here's how to use it with this GraphQL API:
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #fbbf24;">Method:</span> POST

<span style="color: #fbbf24;">URL:</span> http://localhost:4000/graphql
      (or https://api.graphqlbook.store/graphql for production)

<span style="color: #fbbf24;">Headers:</span>
Content-Type: application/json
Authorization: Bearer &lt;your-token&gt;  <span style="color: #52525b;">// Only for authenticated requests</span>

<span style="color: #fbbf24;">Body (JSON):</span>
{
  "query": "{ books { id title price } }"
}</pre>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Using Online Tools</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        You can also use online GraphQL tools like Apollo GraphQL Studio, Insomnia, or similar:
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #fbbf24;">Apollo GraphQL Studio:</span> https://studio.apollographql.com/sandbox/explore
<span style="color: #fbbf24;">Insomnia:</span> https://insomnia.rest/download
<span style="color: #fbbf24;">Postman:</span> https://www.postman.com/downloads

<span style="color: #52525b;">// Simply set the endpoint URL and send your GraphQL query</span></pre>
                    </div>

                    <div style="margin-top: 25px; padding: 20px; background: linear-gradient(135deg, rgba(74, 222, 128, 0.1) 0%, rgba(74, 222, 128, 0.05) 100%); border-radius: 12px; border: 1px solid rgba(74, 222, 128, 0.2);">
                        <div style="display: flex; align-items: center; gap: 15px; flex-wrap: wrap;">
                            <div style="flex: 1; min-width: 200px;">
                                <div style="font-weight: 600; color: rgba(255,255,255,0.9); margin-bottom: 5px;">Ready-to-use Postman Collection</div>
                                <div style="font-size: 0.85rem; color: rgba(255,255,255,0.5);">Import all queries and mutations into Postman with one click</div>
                            </div>
                            <a href="/graphql.json" download style="display: inline-flex; align-items: center; gap: 8px; padding: 12px 24px; background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%); color: #000; text-decoration: none; border-radius: 8px; font-weight: 600; font-size: 0.9rem; transition: transform 0.2s, box-shadow 0.2s;">
                                <svg width="18" height="18" viewBox="0 0 24 24" fill="currentColor"><path d="M19 9h-4V3H9v6H5l7 7 7-7zM5 18v2h14v-2H5z"/></svg>
                                Download Collection
                            </a>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Authentication Flow</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        The API uses JWT (JSON Web Tokens) for authentication. Here's the complete flow:
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #52525b;">// Step 1: Login to get your JWT token</span>
mutation {
  login(username: "admin", password: "password123") {
    success
    token
    user {
      id
      username
      role
    }
  }
}

<span style="color: #52525b;">// Step 2: Use the token in subsequent requests</span>
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...

<span style="color: #52525b;">// Step 3: Token is valid for 6 hours. Include it in the
//         Authorization header for protected endpoints</span></pre>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Default Credentials</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        The API comes with pre-configured test accounts:
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #4ade80;">Admin:</span>   username: admin   password: password123   role: admin
<span style="color: #60a5fa;">Staff:</span>   username: staff   password: password123   role: staff
<span style="color: #fbbf24;">User:</span>    username: user    password: password123   role: user</pre>
                    </div>

                    <div class="doc-nav">
                        <a href="#" class="doc-nav-btn prev" onclick="showSection('docker-install'); return false;">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 18l-6-6 6-6"/></svg>
                            Previous
                        </a>
                        <a href="#" class="doc-nav-btn next" onclick="showSection('data-types'); return false;">
                            Next
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18l6-6-6-6"/></svg>
                        </a>
                    </div>
                </div>

                <!-- Local Installation Section -->
                <div id="local-install" class="doc-section glass-panel">
                    <div class="section-title">Local Installation</div>
                    
                    <p style="color: rgba(255,255,255,0.7); margin-bottom: 20px; line-height: 1.6;">
                        Run the API directly on your machine. Requires C++ compiler, PostgreSQL, and required libraries.
                    </p>

                    <div class="section-title" style="margin-top: 20px;">Prerequisites</div>
                    <ul style="color: rgba(255,255,255,0.6); margin-left: 20px; margin-bottom: 20px; line-height: 1.8;">
                        <li>C++ compiler (g++)</li>
                        <li>PostgreSQL database</li>
                        <li>libpq-dev</li>
                        <li>libjwt-dev</li>
                        <li>libcurl-dev</li>
                    </ul>

                    <div class="section-title" style="margin-top: 20px;">Quick Setup</div>
                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Clone and Build</div>
                            <button class="copy-button" onclick="copyToClipboard('local-setup')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="local-setup">git clone https://github.com/DghostNinja/graphql-bookstore-API.git
cd GraphQL-Bookstore
./build.sh</pre>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 20px;">Run the Server</div>
                    <div class="code-block">
                        <pre style="color: #a3a3a3;">./bookstore-server</pre>
                    </div>

                    <div class="section-title" style="margin-top: 20px;">Server URLs</div>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #4ade80;">DOCS:</span> http://localhost:4000
<span style="color: #60a5fa;">API Endpoint:</span> http://localhost:4000/graphql</pre>
                    </div>

                    <div class="doc-nav">
                        <a href="#" class="doc-nav-btn prev" onclick="showSection('overview'); return false;">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 18l-6-6 6-6"/></svg>
                            Previous
                        </a>
                        <a href="#" class="doc-nav-btn next" onclick="showSection('docker-install'); return false;">
                            Next
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18l6-6-6-6"/></svg>
                        </a>
                    </div>
                </div>

                <!-- Docker Installation Section -->
                <div id="docker-install" class="doc-section glass-panel">
                    <div class="section-title">Docker Installation</div>

                    <p style="color: rgba(255,255,255,0.7); margin-bottom: 20px; line-height: 1.6;">
                        Run the API in a Docker container. Easiest way - handles all dependencies automatically.
                    </p>

                    <div class="section-title" style="margin-top: 20px;">Quick Setup</div>
                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Clone and Run</div>
                            <button class="copy-button" onclick="copyToClipboard('docker-quick')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="docker-quick">git clone https://github.com/DghostNinja/graphql-bookstore-API.git
cd GraphQL-Bookstore
docker-compose up --build</pre>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 20px;">Server URLs</div>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #4ade80;">DOCS:</span> http://localhost:4000
<span style="color: #60a5fa;">API Endpoint:</span> http://localhost:4000/graphql</pre>
                    </div>

                    <div class="section-title" style="margin-top: 20px;">Stop the Server</div>
                    <div class="code-block">
                        <pre style="color: #a3a3a3;">docker-compose down</pre>
                    </div>

                    <div class="doc-nav">
                        <a href="#" class="doc-nav-btn prev" onclick="showSection('local-install'); return false;">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 18l-6-6 6-6"/></svg>
                            Previous
                        </a>
                        <a href="#" class="doc-nav-btn next" onclick="showSection('how-to-send-requests'); return false;">
                            Next
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18l6-6-6-6"/></svg>
                        </a>
                    </div>
                </div>

                <!-- Data Types & Schema Section -->
                <div id="data-types" class="doc-section glass-panel">
                    <div class="section-title">Data Types & Schema</div>
                    
                    <p style="color: rgba(255,255,255,0.7); margin-bottom: 20px; line-height: 1.6;">
                        This section provides an overview of the GraphQL schema, including available data types and how to explore the API using introspection.
                    </p>

                    <div class="section-title" style="margin-top: 25px;">Main Data Types</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        The API defines several core types that represent the bookstore data model:
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #fbbf24;">Book</span>         - Represents a book in the catalog
  id, title, description, price, stockQuantity, categoryId, authorId

<span style="color: #fbbf24;">Author</span>       - Represents a book author
  id, firstName, lastName, biography

<span style="color: #fbbf24;">User</span>         - Represents a registered user
  id, username, email, role, firstName, lastName, phone, city

<span style="color: #fbbf24;">Order</span>        - Represents a customer order
  id, orderNumber, userId, status, totalAmount, createdAt, items

<span style="color: #fbbf24;">OrderItem</span>   - Represents an item in an order
  id, bookId, quantity, price

<span style="color: #fbbf24;">Cart</span>         - Represents a shopping cart
  id, userId, items, subtotal, tax, discount, total

<span style="color: #fbbf24;">CartItem</span>     - Represents an item in the cart
  id, bookId, quantity, price

<span style="color: #fbbf24;">Review</span>       - Represents a book review
  id, bookId, userId, rating, comment, createdAt

<span style="color: #fbbf24;">Webhook</span>      - Represents a webhook subscription
  id, userId, url, events, secret, isActive</pre>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Field Selection</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        GraphQL allows you to request only the specific fields you need. This reduces response size and improves performance.
                    </p>
                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Request All Fields</div>
                            <button class="copy-button" onclick="copyToClipboard('field-all')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="field-all">query {
  books {
    id
    title
    description
    price
    stockQuantity
    author {
      firstName
      lastName
    }
  }
}</pre>
                        </div>
                    </div>
                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Request Only Specific Fields</div>
                            <button class="copy-button" onclick="copyToClipboard('field-specific')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="field-specific">query {
  books {
    id
    title
    price
  }
}</pre>
                        </div>
                    </div>
                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Nested Field Selection</div>
                            <button class="copy-button" onclick="copyToClipboard('field-nested')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="field-nested">query {
  cart {
    id
    items {
      bookId
      quantity
      book {
        title
        price
      }
    }
  }
}</pre>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">GraphQL Introspection</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        GraphQL supports introspection, allowing you to query the schema itself to discover available types, fields, and operations.
                    </p>
                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Query All Types</div>
                            <button class="copy-button" onclick="copyToClipboard('introspect-types')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="introspect-types">query {
  __schema {
    types {
      name
      kind
      description
    }
  }
}</pre>
                        </div>
                    </div>
                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Query Specific Type</div>
                            <button class="copy-button" onclick="copyToClipboard('introspect-type')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="introspect-type">query {
  __type(name: "Book") {
    name
    kind
    fields {
      name
      type {
        name
        kind
      }
    }
  }
}</pre>
                        </div>
                    </div>
                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Query All Queries & Mutations</div>
                            <button class="copy-button" onclick="copyToClipboard('introspect-ops')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="introspect-ops">query {
  __schema {
    queryType { name }
    mutationType { name }
    subscriptionType { name }
  }
}</pre>
                        </div>
                    </div>
                    <div class="code-block-with-copy">
                        <div class="code-header">
                            <div class="code-title">Query Type Fields</div>
                            <button class="copy-button" onclick="copyToClipboard('introspect-fields')">Copy</button>
                        </div>
                        <div class="code-block">
                            <pre id="introspect-fields">query {
  __schema {
    queryType {
      fields {
        name
        description
        type {
          name
        }
        args {
          name
          type {
            name
          }
        }
      }
    }
  }
}</pre>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Common Filters & Arguments</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        Many queries accept arguments to filter, sort, or limit results:
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;"><span style="color: #fbbf24;">books</span>(search: String, categoryId: Int, limit: Int)
  - search: Filter by title or description
  - categoryId: Filter by category
  - limit: Limit number of results

<span style="color: #fbbf24;">book</span>(id: Int!)
  - id: Required - the book ID

<span style="color: #fbbf24;">_searchAdvanced</span>(query: String!)
  - query: Search query string

<span style="color: #fbbf24;">bookReviews</span>(bookId: Int!)
  - bookId: Required - filter reviews by book</pre>
                    </div>

                    <div class="doc-nav">
                        <a href="#" class="doc-nav-btn prev" onclick="showSection('how-to-send-requests'); return false;">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 18l-6-6 6-6"/></svg>
                            Previous
                        </a>
                        <a href="#" class="doc-nav-btn next" onclick="showSection('queries'); return false;">
                            Next
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18l6-6-6-6"/></svg>
                        </a>
                    </div>
                </div>

                <!-- Queries Section -->
                <div id="queries" class="doc-section glass-panel">
                    <div class="section-title">Available Queries</div>
                    
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 20px; line-height: 1.6;">
                        Queries are used to fetch data from the API. Some queries require authentication.
                    </p>

                    <div class="section-title" style="margin-top: 20px;">Public Queries (No Auth Required)</div>
                    <div class="endpoints-grid">
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-optional">No Auth</span></div>
                            <div class="endpoint-name">books</div>
                            <div class="endpoint-desc">List all books with optional search and category filters</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('query-books')">Copy</button>
                                <code id="query-books">{ books { id title price } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-optional">No Auth</span></div>
                            <div class="endpoint-name">book</div>
                            <div class="endpoint-desc">Get details of a specific book by ID</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('query-book')">Copy</button>
                                <code id="query-book">{ book(id: 1) { id title price author { firstName } } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-optional">No Auth</span></div>
                            <div class="endpoint-name">bookReviews</div>
                            <div class="endpoint-desc">Get reviews for a specific book</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('query-book-reviews')">Copy</button>
                                <code id="query-book-reviews">{ bookReviews(bookId: 1) { id rating comment } }</code>
                            </div>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Protected Queries (Auth Required)</div>
                    <div class="endpoints-grid">
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">me</div>
                            <div class="endpoint-desc">Get current authenticated user information</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('query-me')">Copy</button>
                                <code id="query-me">{ me { id username role email } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">cart</div>
                            <div class="endpoint-desc">Get current user's shopping cart</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('query-cart')">Copy</button>
                                <code id="query-cart">{ cart { id items { bookId quantity } totalAmount } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">orders</div>
                            <div class="endpoint-desc">Get current user's order history</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('query-orders')">Copy</button>
                                <code id="query-orders">{ orders { id orderNumber status totalAmount } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">myReviews</div>
                            <div class="endpoint-desc">Get reviews written by current user</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('query-my-reviews')">Copy</button>
                                <code id="query-my-reviews">{ myReviews { id rating comment bookId } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-query">Query</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">webhooks</div>
                            <div class="endpoint-desc">Get current user's registered webhooks</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('query-webhooks')">Copy</button>
                                <code id="query-webhooks">{ webhooks { id url events isActive } }</code>
                            </div>
                        </div>
                    </div>

                    <div class="doc-nav">
                        <a href="#" class="doc-nav-btn prev" onclick="showSection('data-types'); return false;">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 18l-6-6 6-6"/></svg>
                            Previous
                        </a>
                        <a href="#" class="doc-nav-btn next" onclick="showSection('mutations'); return false;">
                            Next
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18l6-6-6-6"/></svg>
                        </a>
                    </div>
                </div>

                <!-- Mutations Section -->
                <div id="mutations" class="doc-section glass-panel">
                    <div class="section-title">Available Mutations</div>
                    
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 20px; line-height: 1.6;">
                        Mutations are used to modify data. Most mutations require authentication.
                    </p>

                    <div class="section-title" style="margin-top: 20px;">Authentication Mutations (No Auth Required)</div>
                    <div class="endpoints-grid">
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-optional">No Auth</span></div>
                            <div class="endpoint-name">register</div>
                            <div class="endpoint-desc">Create a new user account</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-register')">Copy</button>
                                <code id="mut-register">mutation { register(username: "user", firstName: "John", lastName: "Doe", password: "pass123") { success token } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-optional">No Auth</span></div>
                            <div class="endpoint-name">login</div>
                            <div class="endpoint-desc">Authenticate and get JWT token</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-login')">Copy</button>
                                <code id="mut-login">mutation { login(username: "admin", password: "password123") { success token } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-optional">No Auth</span></div>
                            <div class="endpoint-name">logout</div>
                            <div class="endpoint-desc">Logout (discard JWT token client-side)</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-logout')">Copy</button>
                                <code id="mut-logout">mutation { logout { success } }</code>
                            </div>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Cart & Order Mutations (Auth Required)</div>
                    <div class="endpoints-grid">
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">addToCart</div>
                            <div class="endpoint-desc">Add a book to shopping cart</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-add-cart')">Copy</button>
                                <code id="mut-add-cart">mutation { addToCart(bookId: 1, quantity: 2) { success } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">removeFromCart</div>
                            <div class="endpoint-desc">Remove a book from shopping cart</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-remove-cart')">Copy</button>
                                <code id="mut-remove-cart">mutation { removeFromCart(bookId: 1) { success } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">applyCoupon</div>
                            <div class="endpoint-desc">Apply a coupon code to cart</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-coupon')">Copy</button>
                                <code id="mut-coupon">mutation { applyCoupon(code: "DISCOUNT10") { success discount } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">createOrder</div>
                            <div class="endpoint-desc">Create an order from cart (without payment)</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-create-order')">Copy</button>
                                <code id="mut-create-order">mutation { createOrder { success orderId } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">purchaseCart</div>
                            <div class="endpoint-desc">Checkout cart with payment</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-purchase')">Copy</button>
                                <code id="mut-purchase">mutation { purchaseCart(cardNumber: "4111111111111111", expiry: "12/25", cvv: "123") { success orderId } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">cancelOrder</div>
                            <div class="endpoint-desc">Cancel an order</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-cancel')">Copy</button>
                                <code id="mut-cancel">mutation { cancelOrder(orderId: "uuid-here") { success } }</code>
                            </div>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">User & Review Mutations (Auth Required)</div>
                    <div class="endpoints-grid">
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">updateProfile</div>
                            <div class="endpoint-desc">Update user profile information</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-update')">Copy</button>
                                <code id="mut-update">mutation { updateProfile(phone: "1234567890", city: "NYC") { success } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">createReview</div>
                            <div class="endpoint-desc">Create a review for a book</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-review')">Copy</button>
                                <code id="mut-review">mutation { createReview(bookId: 1, rating: 5, comment: "Great book!") { success } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">deleteReview</div>
                            <div class="endpoint-desc">Delete a review</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-delete-review')">Copy</button>
                                <code id="mut-delete-review">mutation { deleteReview(reviewId: 1) { success } }</code>
                            </div>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Webhook Mutations (Auth Required)</div>
                    <div class="endpoints-grid">
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">registerWebhook</div>
                            <div class="endpoint-desc">Register a webhook URL for notifications</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-webhook')">Copy</button>
                                <code id="mut-webhook">mutation { registerWebhook(url: "https://example.com/webhook", events: ["order.created"]) { success } }</code>
                            </div>
                        </div>
                        <div class="endpoint-card">
                            <div class="endpoint-header"><span class="endpoint-method method-mutation">Mutation</span><span class="auth-badge auth-required">Auth</span></div>
                            <div class="endpoint-name">testWebhook</div>
                            <div class="endpoint-desc">Test a registered webhook</div>
                            <div class="code-block" style="margin-top: 8px; font-size: 0.7rem;">
                                <button class="copy-button" onclick="copyToClipboard('mut-test-webhook')">Copy</button>
                                <code id="mut-test-webhook">mutation { testWebhook(webhookId: "uuid-here") { success } }</code>
                            </div>
                        </div>
                    </div>

                    <div class="doc-nav">
                        <a href="#" class="doc-nav-btn prev" onclick="showSection('queries'); return false;">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 18l-6-6 6-6"/></svg>
                            Previous
                        </a>
                        <a href="#" class="doc-nav-btn next" onclick="showSection('vulnerabilities'); return false;">
                            Next
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18l6-6-6-6"/></svg>
                        </a>
                    </div>
                </div>

                <!-- Vulnerabilities Section -->
                <div id="vulnerabilities" class="doc-section glass-panel">
                    <div class="section-title">Security Considerations</div>
                    <div class="code-block">
                        <div style="color: #a3a3a3; line-height: 1.8;">
                            <div style="margin-bottom: 15px;">This API contains various endpoints that may exhibit unexpected behavior under certain conditions:</div>
                            <div style="margin-bottom: 10px;">&#8226; <strong>Authentication</strong> - Response times may vary</div>
                            <div style="margin-bottom: 10px;">&#8226; <strong>Data Access</strong> - Some endpoints may allow broader data access</div>
                            <div style="margin-bottom: 10px;">&#8226; <strong>Input Processing</strong> - Various input fields are processed differently</div>
                            <div style="margin-bottom: 10px;">&#8226; <strong>Batch Operations</strong> - Multiple operations may behave unexpectedly</div>
                            <div style="margin-bottom: 10px;">&#8226; <strong>XML Handling</strong> - Some endpoints accept XML payloads</div>
                            <div style="margin-bottom: 10px;">&#8226; <strong>Debug Endpoints</strong> - Internal information may be exposed</div>
                        </div>
                    </div>

                    <div class="doc-nav">
                        <a href="#" class="doc-nav-btn prev" onclick="showSection('mutations'); return false;">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 18l-6-6 6-6"/></svg>
                            Previous
                        </a>
                        <a href="#" class="doc-nav-btn next" onclick="showSection('mcp-docs'); return false;">
                            Next
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18l6-6-6-6"/></svg>
                        </a>
                    </div>
                </div>

                <!-- MCP Server Section -->
                <div id="mcp-docs" class="doc-section glass-panel">
                    <div class="section-title">MCP Server - AI/LLM Integration</div>
                    
                    <p style="color: rgba(255,255,255,0.7); margin-bottom: 20px; line-height: 1.6;">
                        The Bookstore API includes an <strong>MCP (Model Context Protocol)</strong> server that exposes all API operations as tools for AI and LLM models. This enables AI assistants to interact with the Bookstore API natively.
                    </p>

                    <div class="section-title" style="margin-top: 25px;">Quick Installation</div>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;">
# Navigate to MCP directory and install
cd mcp
npm install</pre>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Usage</div>
                    
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 10px; line-height: 1.6;">
                        <strong>Local Development:</strong>
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;">
# Start the Bookstore API server (from project root)
./bookstore-server

# In another terminal, start MCP server
cd mcp
npm start</pre>
                    </div>

                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 10px; line-height: 1.6;">
                        <strong>Production (Live API):</strong>
                    </p>
                    <div class="code-block">
                        <pre style="color: #a3a3a3; line-height: 1.8;">
cd mcp
API_URL=https://api.graphqlbook.store/graphql npm start</pre>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Available MCP Tools</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 15px; line-height: 1.6;">
                        The MCP server exposes 25 tools organized by category:
                    </p>
                    
                    <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(160px, 1fr)); gap: 10px; margin-bottom: 20px;">
                        <div style="background: rgba(255,255,255,0.05); padding: 12px; border-radius: 8px;">
                            <div style="color: #4fd1c5; font-weight: bold; margin-bottom: 6px; font-size: 14px;">Authentication</div>
                            <div style="color: rgba(255,255,255,0.6); font-size: 11px; line-height: 1.5;">
                                bookstore_login<br>
                                bookstore_register<br>
                                bookstore_me
                            </div>
                        </div>
                        <div style="background: rgba(255,255,255,0.05); padding: 12px; border-radius: 8px;">
                            <div style="color: #4fd1c5; font-weight: bold; margin-bottom: 6px; font-size: 14px;">Books</div>
                            <div style="color: rgba(255,255,255,0.6); font-size: 11px; line-height: 1.5;">
                                bookstore_books<br>
                                bookstore_book<br>
                                bookstore_search
                            </div>
                        </div>
                        <div style="background: rgba(255,255,255,0.05); padding: 12px; border-radius: 8px;">
                            <div style="color: #4fd1c5; font-weight: bold; margin-bottom: 6px; font-size: 14px;">Cart</div>
                            <div style="color: rgba(255,255,255,0.6); font-size: 11px; line-height: 1.5;">
                                bookstore_cart<br>
                                bookstore_add_to_cart<br>
                                bookstore_remove_from_cart
                            </div>
                        </div>
                        <div style="background: rgba(255,255,255,0.05); padding: 12px; border-radius: 8px;">
                            <div style="color: #4fd1c5; font-weight: bold; margin-bottom: 6px; font-size: 14px;">Orders</div>
                            <div style="color: rgba(255,255,255,0.6); font-size: 11px; line-height: 1.5;">
                                bookstore_orders<br>
                                bookstore_create_order<br>
                                bookstore_purchase_cart<br>
                                bookstore_cancel_order
                            </div>
                        </div>
                        <div style="background: rgba(255,255,255,0.05); padding: 12px; border-radius: 8px;">
                            <div style="color: #4fd1c5; font-weight: bold; margin-bottom: 6px; font-size: 14px;">Reviews</div>
                            <div style="color: rgba(255,255,255,0.6); font-size: 11px; line-height: 1.5;">
                                bookstore_create_review<br>
                                bookstore_delete_review<br>
                                bookstore_book_reviews<br>
                                bookstore_my_reviews
                            </div>
                        </div>
                        <div style="background: rgba(255,255,255,0.05); padding: 12px; border-radius: 8px;">
                            <div style="color: #4fd1c5; font-weight: bold; margin-bottom: 6px; font-size: 14px;">Other</div>
                            <div style="color: rgba(255,255,255,0.6); font-size: 11px; line-height: 1.5;">
                                bookstore_update_profile<br>
                                bookstore_apply_coupon<br>
                                bookstore_webhooks<br>
                                bookstore_admin_stats
                            </div>
                        </div>
                    </div>

                    <div class="section-title" style="margin-top: 25px;">Claude Desktop Integration</div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 10px; line-height: 1.6;">
                        Add the following to your Claude Desktop config file:
                    </p>
                    <div class="code-block" style="overflow-x: auto;">
                        <pre style="color: #a3a3a3; line-height: 1.6; white-space: pre-wrap; word-wrap: break-word;">
{
  "mcpServers": {
    "bookstore": {
      "command": "node",
      "args": ["/path/to/GraphQL-Bookstore/mcp/mcp_server.mjs"],
      "env": {
        "API_URL": "http://localhost:4000/graphql"
      }
    }
  }
}</pre>
                    </div>
                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 20px; line-height: 1.6;">
                        For production, change <code style="background: rgba(255,255,255,0.1); padding: 2px 6px; border-radius: 3px;">API_URL</code> to <code style="background: rgba(255,255,255,0.1); padding: 2px 6px; border-radius: 3px;">https://api.graphqlbook.store/graphql</code>.
                    </p>

                    <div class="section-title" style="margin-top: 25px;">Environment Variables</div>
                    <div style="overflow-x: auto; margin-bottom: 20px;">
                        <table style="width: 100%; min-width: 300px; border-collapse: collapse;">
                        <tr style="border-bottom: 1px solid rgba(255,255,255,0.1);">
                            <th style="text-align: left; padding: 10px; color: #4fd1c5;">Variable</th>
                            <th style="text-align: left; padding: 10px; color: #4fd1c5;">Default</th>
                            <th style="text-align: left; padding: 10px; color: #4fd1c5;">Description</th>
                        </tr>
                        <tr style="border-bottom: 1px solid rgba(255,255,255,0.1);">
                            <td style="padding: 10px; color: rgba(255,255,255,0.8);">API_URL</td>
                            <td style="padding: 10px; color: rgba(255,255,255,0.6);">http://localhost:4000/graphql</td>
                            <td style="padding: 10px; color: rgba(255,255,255,0.6);">Bookstore API endpoint</td>
                        </tr>
                    </table>
                    </div>

                    <p style="color: rgba(255,255,255,0.6); margin-bottom: 20px; line-height: 1.6;">
                        See <code style="background: rgba(255,255,255,0.1); padding: 2px 6px; border-radius: 3px;">mcp/README.md</code> for detailed documentation.
                    </p>

                    <div class="doc-nav">
                        <a href="#" class="doc-nav-btn prev" onclick="showSection('vulnerabilities'); return false;">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 18l-6-6 6-6"/></svg>
                            Previous
                        </a>
                        <a href="#" class="doc-nav-btn next" onclick="showSection('feedback'); return false;">
                            Next
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18l6-6-6-6"/></svg>
                        </a>
                    </div>
                </div>

                <!-- Feedback Section -->
                <div id="feedback" class="doc-section glass-panel">
                    <div class="section-title">Feedback & Comments</div>
                    
                    <p style="color: rgba(255,255,255,0.7); margin-bottom: 20px; line-height: 1.6;">
                        Have suggestions, found bugs, or want to share your experience? We'd love to hear from you! Your feedback helps improve this API.
                    </p>

                    <div class="feedback-form">
                        <input type="text" id="feedbackHoneypot" class="honeypot-field" tabindex="-1" autocomplete="off" />
                        <div class="form-group">
                            <label for="feedbackName">Your Name</label>
                            <input type="text" id="feedbackName" placeholder="Enter your name (optional)" maxlength="100" />
                        </div>
                        <div class="form-group">
                            <label for="feedbackComment">Your Comment</label>
                            <textarea id="feedbackComment" placeholder="Share your thoughts, suggestions, or report issues..." rows="5" maxlength="1000"></textarea>
                        </div>
                        <div class="form-group">
                            <label for="feedbackCaptcha">What is <span id="captchaNum1"></span> + <span id="captchaNum2"></span>?</label>
                            <input type="number" id="feedbackCaptcha" placeholder="Enter the sum" style="width: 200px;" />
                        </div>
                        <button class="submit-button" onclick="submitFeedback()">Submit Feedback</button>
                        <div id="feedbackMessage" class="feedback-message"></div>
                    </div>

                    <div class="doc-nav">
                        <a href="#" class="doc-nav-btn prev" onclick="showSection('mcp-docs'); return false;">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M15 18l-6-6 6-6"/></svg>
                            Previous
                        </a>
                        <a href="#" class="doc-nav-btn next" onclick="return false;" style="visibility: hidden;">
                            Next
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18l6-6-6-6"/></svg>
                        </a>
                    </div>
                </div>

                </div>
            </div>
        </div>

    <footer>
        <div class="footer-content">
            <div class="footer-version">API v1.0</div>
            <div class="footer-brand">GraphQL Bookstore API</div>
            <div class="footer-links">
                <a href="https://github.com/DghostNinja" target="_blank" title="GitHub">
                    <svg width="20" height="20" viewBox="0 0 24 24" fill="currentColor"><path d="M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z"/></svg>
                </a>
                <a href="https://www.linkedin.com/in/shonde-samuel" target="_blank" title="LinkedIn">
                    <svg width="20" height="20" viewBox="0 0 24 24" fill="currentColor"><path d="M19 0h-14c-2.761 0-5 2.239-5 5v14c0 2.761 2.239 5 5 5h14c2.762 0 5-2.239 5-5v-14c0-2.761-2.238-5-5-5zm-11 19h-3v-11h3v11zm-1.5-12.268c-.966 0-1.75-.79-1.75-1.764s.784-1.764 1.75-1.764 1.75.79 1.75 1.764-.783 1.764-1.75 1.764zm13.5 12.268h-3v-5.604c0-3.368-4-3.113-4 0v5.604h-3v-11h3v1.765c1.396-2.586 7-2.777 7 2.476v6.759z"/></svg>
                </a>
                <a href="https://x.com/Dghost_Ninja" target="_blank" title="X (Twitter)">
                    <svg width="20" height="20" viewBox="0 0 24 24" fill="currentColor"><path d="M18.244 2.25h3.308l-7.227 8.26 8.502 11.24H16.17l-5.214-6.817L4.99 21.75H1.68l7.73-8.835L1.254 2.25H8.08l4.713 6.231zm-1.161 17.52h1.833L7.084 4.126H5.117z"/></svg>
                </a>
            </div>
            <div class="footer-refs">
                Built with C++17 &bull; PostgreSQL Database &bull; JWT Authentication &bull; RESTful webhook integration
                <br>
                &copy; 2026 GraphQL Bookstore. All rights reserved.
            </div>
        </div>
    </footer>

    <script>
        var captchaNum1 = 0;
        var captchaNum2 = 0;

        function generateCaptcha() {
            captchaNum1 = Math.floor(Math.random() * 10) + 1;
            captchaNum2 = Math.floor(Math.random() * 10) + 1;
            document.getElementById('captchaNum1').textContent = captchaNum1;
            document.getElementById('captchaNum2').textContent = captchaNum2;
            document.getElementById('feedbackCaptcha').value = '';
        }

        function sanitizeInput(str) {
            const div = document.createElement('div');
            div.textContent = str;
            return div.innerHTML;
        }

        function submitFeedback() {
            const honeypot = document.getElementById('feedbackHoneypot').value;
            const name = sanitizeInput(document.getElementById('feedbackName').value.trim());
            const comment = sanitizeInput(document.getElementById('feedbackComment').value.trim());
            const captchaAnswer = document.getElementById('feedbackCaptcha').value;
            const messageEl = document.getElementById('feedbackMessage');
            const submitBtn = document.querySelector('.submit-button');

            if (honeypot) {
                return;
            }

            const now = Date.now();
            const lastSubmit = parseInt(localStorage.getItem('feedbackLastSubmit') || '0');
            if (now - lastSubmit < 30000) {
                messageEl.className = 'feedback-message error';
                messageEl.textContent = 'Please wait 30 seconds before submitting again.';
                messageEl.style.display = 'block';
                return;
            }

            if (!comment) {
                messageEl.className = 'feedback-message error';
                messageEl.textContent = 'Please enter a comment before submitting.';
                messageEl.style.display = 'block';
                return;
            }

            if (comment.length < 3) {
                messageEl.className = 'feedback-message error';
                messageEl.textContent = 'Comment must be at least 3 characters.';
                messageEl.style.display = 'block';
                return;
            }

            const expectedAnswer = captchaNum1 + captchaNum2;
            if (parseInt(captchaAnswer) !== expectedAnswer) {
                messageEl.className = 'feedback-message error';
                messageEl.textContent = 'Incorrect answer. Please solve the math problem.';
                messageEl.style.display = 'block';
                generateCaptcha();
                return;
            }

            submitBtn.disabled = true;
            submitBtn.textContent = 'Submitting...';
            messageEl.style.display = 'none';

            const formUrl = 'https://script.google.com/macros/s/AKfycbwKpb-m9fqrNY8ZpdLQfiAYWrvCzzHgaAEIQQfWV7o4srGT8YrskPNcDU2Y4dLa-TU_/exec';

            fetch(formUrl, {
                method: 'POST',
                mode: 'no-cors',
                headers: {
                    'Content-Type': 'text/plain;charset=utf-8',
                },
                body: JSON.stringify({
                    name: name,
                    comment: comment
                })
            }).then(() => {
                localStorage.setItem('feedbackLastSubmit', now.toString());
                messageEl.className = 'feedback-message success';
                messageEl.textContent = 'Thank you for your feedback!';
                messageEl.style.display = 'block';
                document.getElementById('feedbackName').value = '';
                document.getElementById('feedbackComment').value = '';
                generateCaptcha();
            }).catch((error) => {
                messageEl.className = 'feedback-message error';
                messageEl.textContent = 'Failed to submit feedback. Please try again.';
                messageEl.style.display = 'block';
            }).finally(() => {
                submitBtn.disabled = false;
                submitBtn.textContent = 'Submit Feedback';
            });
        }

        generateCaptcha();
    </script>

    <script>
        var token = localStorage.getItem("token") || "";

        var queries = {
            1: "query { books(limit: 10) { id title author { firstName lastName } price stockQuantity } }",
            2: "query { book(id: 1) { id title description price stockQuantity author { firstName lastName } } }",
            3: "query { me { id username email role firstName lastName } }",
            4: "query { cart { items { id book { title } quantity price } totalAmount } }",
            5: "query { orders { id orderNumber status totalAmount createdAt } }",
            6: "query { _searchAdvanced(query: \"1 OR 1=1\") { id title } }",
            7: "query { _internalUserSearch(username: \"a\") { id username role email } }",
            8: "mutation { register(username: \"newuser\", firstName: \"John\", lastName: \"Doe\", password: \"pass123\") { success message token user { id username } } }",
            9: "mutation { login(username: \"admin\", password: \"password123\") { success token user { id username role } } }",
            10: "mutation { addToCart(bookId: 1, quantity: 2) { success message } }",
            11: "mutation { createOrder { success orderId totalAmount } }",
            12: "mutation { cancelOrder(orderId: \"uuid-here\") { success message } }",
            13: "mutation { createReview(bookId: 1, rating: 5, comment: \"Great book!\") { success message } }",
            14: "mutation { deleteReview(reviewId: 1) { success message } }",
            15: "mutation { updateProfile(firstName: \"New\", lastName: \"Name\") { success message } }",
            16: "query { _fetchExternalResource(url: \"http://example.com\") { content } }",
            17: "mutation { registerWebhook(url: \"http://example.com/webhook\", events: [\"order.created\"], secret: \"secret123\") { success message webhook { id } } }",
            18: "mutation { testWebhook(webhookId: \"uuid-here\") { success message } }"
        };

        function setQuery(id) {
            if (queries[id]) {
                document.getElementById("queryInput").value = queries[id];
                document.getElementById("queryStatusMsg").style.display = "none";
            }
        }

        function getAuthHeader() {
            if (token) return "Bearer " + token;
            return "";
        }

        function showQueryStatus(msg, type) {
            var el = document.getElementById("queryStatusMsg");
            el.textContent = msg;
            el.className = "status-msg status-" + type;
            el.style.display = "block";
        }

        function showLoginStatus(msg, type) {
            var el = document.getElementById("loginStatusMsg");
            el.textContent = msg;
            el.className = "status-msg status-" + type;
            el.style.display = "block";
        }

        function showRegStatus(msg, type) {
            var el = document.getElementById("regStatusMsg");
            el.textContent = msg;
            el.className = "status-msg status-" + type;
            el.style.display = "block";
        }

        function runQuery() {
            var query = document.getElementById("queryInput").value.trim();
            if (!query) {
                document.getElementById("responseArea").textContent = "Please enter a query first.";
                return;
            }
            document.getElementById("responseArea").textContent = "Loading...";
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/graphql", true);
            xhr.setRequestHeader("Content-Type", "application/json");
            var auth = getAuthHeader();
            if (auth) {
                xhr.setRequestHeader("Authorization", auth);
            }
            xhr.onreadystatechange = function() {
                if (xhr.readyState === 4) {
                    var resp = document.getElementById("responseArea");
                    if (xhr.status === 200) {
                        try {
                            var json = JSON.parse(xhr.responseText);
                            resp.textContent = JSON.stringify(json, null, 2);
                        } catch (e) {
                            resp.textContent = xhr.responseText;
                        }
                    } else {
                        resp.textContent = "Error " + xhr.status + ": " + xhr.statusText + "\n" + xhr.responseText;
                    }
                }
            };
            xhr.onerror = function() {
                document.getElementById("responseArea").textContent = "Network error - is the server running?";
            };
            xhr.send(JSON.stringify({ query: query }));
        }

        function doLogin() {
            var user = document.getElementById("loginUsername").value;
            var pass = document.getElementById("loginPassword").value;
            if (!user || !pass) { showLoginStatus("Enter username and password", "error"); return; }
            var q = "mutation { login(username: \"" + user + "\", password: \"" + pass + "\") { success token message } }";
            showLoginStatus("Logging in...", "loading");
            fetch("/graphql", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ query: q })
            })
            .then(function(r) { return r.json(); })
            .then(function(d) {
                var disp = document.getElementById("loginTokenDisplay");
                if (d.data && d.data.login && d.data.login.success) {
                    token = d.data.login.token;
                    localStorage.setItem("token", token);
                    disp.textContent = "Token: " + token.substring(0, 25) + "...";
                    disp.style.display = "block";
                    showLoginStatus("Login successful!", "success");
                } else {
                    disp.style.display = "none";
                    showLoginStatus("Login failed: " + (d.data && d.data.login ? d.data.login.message : "Unknown error"), "error");
                }
                setTimeout(function() { document.getElementById("loginStatusMsg").style.display = "none"; }, 3000);
            })
            .catch(function(err) { showLoginStatus("Error: " + err, "error"); });
        }

        function doRegister() {
            var user = document.getElementById("regUsername").value;
            var pass = document.getElementById("regPassword").value;
            var fname = document.getElementById("regFirstName").value;
            var lname = document.getElementById("regLastName").value;
            if (!user || !pass || !fname || !lname) { showRegStatus("Fill all fields", "error"); return; }
            var q = "mutation { register(username: \"" + user + "\", password: \"" + pass + "\", firstName: \"" + fname + "\", lastName: \"" + lname + "\") { success message token user { id username } } }";
            showRegStatus("Registering...", "loading");
            fetch("/graphql", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({ query: q })
            })
            .then(function(r) { return r.json(); })
            .then(function(d) {
                var disp = document.getElementById("regTokenDisplay");
                if (d.data && d.data.register && d.data.register.success) {
                    token = d.data.register.token;
                    localStorage.setItem("token", token);
                    disp.textContent = "Token: " + token.substring(0, 25) + "...";
                    disp.style.display = "block";
                    showRegStatus("Registration successful!", "success");
                } else {
                    disp.style.display = "none";
                    showRegStatus("Registration failed: " + (d.data && d.data.register ? d.data.register.message : "Unknown error"), "error");
                }
                setTimeout(function() { document.getElementById("regStatusMsg").style.display = "none"; }, 3000);
            })
            .catch(function(err) { showRegStatus("Error: " + err, "error"); });
        }

        function clearQuery() {
            document.getElementById("queryInput").value = "";
            document.getElementById("responseArea").textContent = "Response will appear here...";
        }

        function loadSample() {
            document.getElementById("queryInput").value = 'query { books { id title author { firstName lastName } price stockQuantity } }';
        }

        function copyApiLink() {
            var link = "http://api.graphqlbook.store/graphql";
            navigator.clipboard.writeText(link).then(function() {
                var el = document.getElementById("apiLink");
                el.textContent = "Copied!";
                el.classList.add("copied");
                setTimeout(function() {
                    el.textContent = "api.graphqlbook.store/graphql";
                    el.classList.remove("copied");
                }, 2000);
            });
        }

        var currentVulnIndex = 0;
        var vulnCards = [];
        var vulnDots = [];
        
        function initVulnSlideshow() {
            vulnCards = document.querySelectorAll('.vuln-card');
            vulnDots = document.querySelectorAll('.vuln-dot');
            if (vulnCards.length > 0) {
                currentVulnIndex = Math.floor(Math.random() * vulnCards.length);
                updateVulnDisplay();
                setInterval(rotateVulnCard, 4000);
            }
        }
        
        var currentCouponIndex = 0;
        var couponCards = [];
        var couponDots = [];
        
        function initCouponCarousel() {
            couponCards = document.querySelectorAll('.coupon-card');
            couponDots = document.querySelectorAll('.coupon-dot');
            if (couponCards.length > 0) {
                currentCouponIndex = Math.floor(Math.random() * couponCards.length);
                updateCouponDisplay();
                setInterval(rotateCouponCard, 5000);
            }
        }
        
        function rotateCouponCard() {
            currentCouponIndex = (currentCouponIndex + 1) % couponCards.length;
            updateCouponDisplay();
        }
        
        function showCouponCard(index) {
            currentCouponIndex = index;
            updateCouponDisplay();
        }
        
        function updateCouponDisplay() {
            couponCards.forEach(function(card, i) {
                card.classList.remove('active');
            });
            couponDots.forEach(function(dot, i) {
                dot.classList.remove('active');
            });
            if (couponCards[currentCouponIndex]) {
                couponCards[currentCouponIndex].classList.add('active');
            }
            if (couponDots[currentCouponIndex]) {
                couponDots[currentCouponIndex].classList.add('active');
            }
        }
        
        function copyCoupon(code) {
            navigator.clipboard.writeText(code).then(function() {
                showQueryStatus('Coupon code copied: ' + code, 'success');
            }).catch(function() {
                showQueryStatus('Failed to copy code', 'error');
            });
        }
        
        function rotateVulnCard() {
            currentVulnIndex = (currentVulnIndex + 1) % vulnCards.length;
            updateVulnDisplay();
        }
        
        function showVulnCard(index) {
            currentVulnIndex = index;
            updateVulnDisplay();
        }
        
        function updateVulnDisplay() {
            vulnCards.forEach(function(card, i) {
                card.classList.remove('active');
            });
            vulnDots.forEach(function(dot, i) {
                dot.classList.remove('active');
            });
            if (vulnCards[currentVulnIndex]) {
                vulnCards[currentVulnIndex].classList.add('active');
            }
            if (vulnDots[currentVulnIndex]) {
                vulnDots[currentVulnIndex].classList.add('active');
            }
        }

        function logout() {
            token = "";
            localStorage.removeItem("token");
            document.getElementById("loginTokenDisplay").style.display = "none";
            document.getElementById("regTokenDisplay").style.display = "none";
            showQueryStatus("Logged out", "info");
            setTimeout(function() { document.getElementById("queryStatusMsg").style.display = "none"; }, 1500);
        }

        document.getElementById("queryInput").addEventListener("keydown", function(e) {
            if (e.key === "Enter" && e.ctrlKey) runQuery();
        });

        function showPage(page) {
            // Hide all pages
            document.getElementById("homePage").style.display = "none";
            document.getElementById("docsPage").style.display = "none";
            
            // Hide all doc sections
            var sections = document.querySelectorAll(".doc-section");
            sections.forEach(function(section) {
                section.classList.remove("active");
            });
            
            // Remove active class from all nav links
            var navLinks = document.querySelectorAll(".nav-link");
            navLinks.forEach(function(link) {
                link.classList.remove("active");
            });
            
            // Show selected page and activate nav link
            if (page === "home") {
                document.getElementById("homePage").style.display = "block";
                navLinks[0].classList.add("active");
            } else if (page === "docs") {
                document.getElementById("docsPage").style.display = "block";
                navLinks[1].classList.add("active");
                // Show overview section by default when docs page opens
                showSection('overview');
            }
        }

        function toggleSidebar() {
            var sidebar = document.getElementById("sidebar");
            var overlay = document.getElementById("sidebarOverlay");
            sidebar.classList.toggle("active");
            overlay.classList.toggle("active");
        }

        function showSection(sectionId) {
            // Hide all doc sections
            var sections = document.querySelectorAll(".doc-section");
            sections.forEach(function(section) {
                section.classList.remove("active");
            });
            
            // Show selected section
            var targetSection = document.getElementById(sectionId);
            if (targetSection) {
                targetSection.classList.add("active");
            }
            
            // Update sidebar active state
            var sidebarItems = document.querySelectorAll(".sidebar-item");
            sidebarItems.forEach(function(item) {
                item.classList.remove("active");
            });
            
            // Find and activate the corresponding sidebar item using exact match
            var activeItem = null;
            for (var i = 0; i < sidebarItems.length; i++) {
                var item = sidebarItems[i];
                var onclickAttr = item.getAttribute('onclick');
                if (onclickAttr && onclickAttr.indexOf('showSection(\'' + sectionId + '\')') !== -1) {
                    activeItem = item;
                    break;
                }
            }
            
            if (activeItem) {
                activeItem.classList.add("active");
            }
            
            // Close sidebar after selection
            var sidebar = document.getElementById("sidebar");
            var overlay = document.getElementById("sidebarOverlay");
            if (sidebar) sidebar.classList.remove("active");
            if (overlay) overlay.classList.remove("active");
            
            // Scroll to the section
            setTimeout(function() {
                targetSection.scrollIntoView({ behavior: 'smooth', block: 'start' });
            }, 100);
        }

        function showDocsAndFeedback() {
            var docsPage = document.getElementById("docsPage");
            var homePage = document.getElementById("homePage");
            
            // If we're on home page, switch to docs first
            if (homePage && homePage.style.display !== "none") {
                homePage.style.display = "none";
                docsPage.style.display = "block";
                
                var navLinks = document.querySelectorAll(".nav-link");
                navLinks.forEach(function(link) {
                    link.classList.remove("active");
                });
                if (navLinks[1]) {
                    navLinks[1].classList.add("active");
                }
            }
            
            // Then show feedback section
            showSection('feedback');
        }

        function copyToClipboard(elementId) {
            var element = document.getElementById(elementId);
            if (element) {
                var text = element.textContent || element.innerText;
                navigator.clipboard.writeText(text).then(function() {
                    // Show feedback
                    var button = event.target;
                    var originalText = button.textContent;
                    button.textContent = "✓ Copied!";
                    button.style.background = "linear-gradient(135deg, rgba(74, 222, 128, 0.3) 0%, rgba(34, 197, 94, 0.3) 100%)";
                    button.style.borderColor = "rgba(74, 222, 128, 0.5)";
                    
                    setTimeout(function() {
                        button.textContent = originalText;
                        button.style.background = "";
                        button.style.borderColor = "";
                    }, 2000);
                }).catch(function(err) {
                    // Fallback for older browsers
                    var textArea = document.createElement("textarea");
                    textArea.value = text;
                    document.body.appendChild(textArea);
                    textArea.select();
                    document.execCommand('copy');
                    document.body.removeChild(textArea);
                    
                    var button = event.target;
                    button.textContent = "✓ Copied!";
                    setTimeout(function() {
                        button.textContent = originalText;
                    }, 2000);
                });
            }
        }

        // Close sidebar when clicking outside
        document.addEventListener('click', function(event) {
            var sidebar = document.getElementById("sidebar");
            var hamburger = document.querySelector(".hamburger");
            
            if (sidebar.classList.contains("active") && 
                !sidebar.contains(event.target) && 
                !hamburger.contains(event.target)) {
                sidebar.classList.remove("active");
            }
        });

        window.addEventListener('load', function() {
            initVulnSlideshow();
            initCouponCarousel();
        });
    </script>
</body>
</html>
)HTMLEOF";
}

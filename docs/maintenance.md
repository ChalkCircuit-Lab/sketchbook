# Maintenance

- Preserve originals in `archive/originals/`.
- Do not edit archived originals directly unless fixing unsafe files.
- Make a cleaned version before rewriting.
- Every project needs a metadata entry in `data/projects.json`.
- Every chapter needs a README.
- Every polished demo should have a short lesson page.
- Every exercise should state prerequisites.
- Backend examples should include run instructions when possible.
- Do not mix product code with sketchbook experiments unless clearly marked.
- Document whether Git history was preserved.
- Keep `.gitignore` broad but not destructive.
- Do not commit secrets, cookies, tokens, `.env` files, browser profiles,
  generated credentials, or private keys.
- `LOGIN` and `COOKIE` require manual security review before import because
  their names suggest auth/session/cookie-related code:
  `https://github.com/ChalkCircuit-Lab/LOGIN.git` and
  `https://github.com/ChalkCircuit-Lab/COOKIE.git`.

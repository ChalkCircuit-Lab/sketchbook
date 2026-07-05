# Import Map

This table records where each old repository was imported, whether Git history
was preserved, and where a future cleaned teaching version should live.

| Original repo | Source URL | New original location | Clean target location | History preserved? | Status | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| bootstrap_demo_5 | https://github.com/ChalkCircuit-Lab/bootstrap_demo_5.git | `archive/originals/bootstrap_demo_5` | `frontend/bootstrap/bootstrap-demo-5` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| bootstrap_demo_8 | https://github.com/ChalkCircuit-Lab/bootstrap_demo_8.git | `archive/originals/bootstrap_demo_8` | `frontend/bootstrap/bootstrap-demo-8` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| bootstrap_avatar_cards_12 | https://github.com/ChalkCircuit-Lab/bootstrap_avatar_cards_12.git | `archive/originals/bootstrap_avatar_cards_12` | `frontend/bootstrap/avatar-cards` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| colored_sections | https://github.com/ChalkCircuit-Lab/colored_sections.git | `archive/originals/colored_sections` | `frontend/html-css/colored-sections` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| clock-navbar | https://github.com/ChalkCircuit-Lab/clock-navbar.git | `archive/originals/clock-navbar` | `frontend/javascript/clock-navbar` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| React-Vite-demo | https://github.com/ChalkCircuit-Lab/React-Vite-demo.git | `archive/originals/React-Vite-demo` | `frontend/react/react-vite-demo` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| disney-personality-quiz | https://github.com/ChalkCircuit-Lab/disney-personality-quiz.git | `archive/originals/disney-personality-quiz` | `frontend/javascript/personality-quiz` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| Coffee-Lover | https://github.com/ChalkCircuit-Lab/Coffee-Lover.git | `archive/originals/Coffee-Lover` | `frontend/html-css/coffee-lover` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| JavaTech-Foundations | https://github.com/ChalkCircuit-Lab/JavaTech-Foundations.git | `archive/originals/JavaTech-Foundations` | `backend/java/javatech-foundations` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| Java-Foundations | https://github.com/ChalkCircuit-Lab/Java-Foundations.git | `archive/originals/Java-Foundations` | `backend/java/java-foundations` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| javatech.lab02.config | https://github.com/ChalkCircuit-Lab/javatech.lab02.config.git | `archive/originals/javatech.lab02.config` | `backend/java/lab02-config` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| Concurrent-Distributed-Labs | https://github.com/ChalkCircuit-Lab/Concurrent-Distributed-Labs.git | `archive/originals/Concurrent-Distributed-Labs` | `backend/distributed-systems/concurrent-distributed-labs` | yes | imported with subtree history | Imported from `main` without `--squash`; three tracked `server.key` private-key files were removed from the current tree and from sketchbook repository history before publishing. |
| Zoo-Database | https://github.com/ChalkCircuit-Lab/Zoo-Database.git | `archive/originals/Zoo-Database` | `backend/databases/zoo-database` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| Zoo-Interface | https://github.com/ChalkCircuit-Lab/Zoo-Interface.git | `archive/originals/Zoo-Interface` | `backend/databases/zoo-interface` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| Route-Seeker | https://github.com/ChalkCircuit-Lab/Route-Seeker.git | `archive/originals/Route-Seeker` | `backend/algorithms/route-seeker` | yes | imported with subtree history | Imported from `main` without `--squash`. |
| LOGIN | https://github.com/ChalkCircuit-Lab/LOGIN.git | Not imported | Not assigned | no | manual review required | Requires manual security review before import because the name suggests auth/session-related code. |
| COOKIE | https://github.com/ChalkCircuit-Lab/COOKIE.git | Not imported | Not assigned | no | manual review required | Requires manual security review before import because the name suggests cookie/session-related code. |

## Import Rule

If another source repository is found later, prefer a history-preserving import:

```bash
git remote add old-bootstrap-demo-5 <OLD_REPO_URL>
git fetch old-bootstrap-demo-5
git subtree add --prefix=archive/originals/bootstrap_demo_5 old-bootstrap-demo-5 main
```

Use `master` instead of `main` if that is the source branch. Do not use
`--squash` unless preserving the full history is too fragile or cumbersome. If a
subtree import fails repeatedly, copy the files into
`archive/originals/<repo-name>` and update this table with the reason history
was not preserved.

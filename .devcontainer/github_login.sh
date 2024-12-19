if ! gh auth status >/dev/null 2>&1; then 
  git config --global credential.helper '!gh auth git-credential'
  gh auth login
else
  echo 'Already logged in'
fi